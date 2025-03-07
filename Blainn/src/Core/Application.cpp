#include "Application.h"

#include "pch.h"

#include "Input.h"
#include "Util/ComboboxSelector.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#include <iostream>

extern const int g_NumFrameResources;

namespace Blainn
{
	Application* Application::s_Instance = nullptr;

	Application::Application(HINSTANCE hInstance, const ApplicationDesc& description)
	{
		if (s_Instance)
		{
			MessageBox(nullptr, L"Tried to instantiate a second application", L"Error!", MB_OK);
			return;
		}

		m_hInstance = hInstance;
		m_AppDescription = description;
		m_ClientWidth = description.WindowWidth;
		m_ClientHeight = description.WindowHeight;
		s_Instance = this;

	}

	Application::~Application()
	{
	}

	bool Application::Initialize()
	{
		WindowDesc windowDesc = {};
		windowDesc.Title = m_AppDescription.Name;
		windowDesc.Width = m_AppDescription.WindowWidth;
		windowDesc.Height = m_AppDescription.WindowHeight;
		windowDesc.Decorated = m_AppDescription.WindowDecorated;
		windowDesc.Fullscreen = m_AppDescription.Fullscreen;
		windowDesc.VSync = m_AppDescription.VSync;

		m_Window = std::shared_ptr<Window>(Window::Create(windowDesc));
		m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

		if (!m_Window->Init())
			return false;

		m_RenderingContext = std::make_shared<DXRenderingContext>();
		m_RenderingContext->Init(m_Window);

		m_ResourceManager = std::make_shared<DXResourceManager>(
			m_RenderingContext->GetDevice()->Device(),
			m_RenderingContext->GetCommandQueue()
		);

		m_RenderingContext->CreateResources();

		m_Scene = std::make_shared<Scene>();

		m_bPaused = false;
		OnResize();

		return true;
	}

	int Application::Run()
	{
		OnInit();
		MSG msg = { nullptr };

		m_Timer.Reset();

		while (msg.message != WM_QUIT)
		{

			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				m_Timer.Tick();

				if (!m_bPaused)
				{
					CalculateFrameStats();
					Update(m_Timer);
					Draw(m_Timer);
				}
				else
					Sleep(10);
			}
		}
		return (int)msg.wParam;
	}

	void Application::Close()
	{
		PostQuitMessage(0);
	}

	void Application::OnResize()
	{
		if(m_ClientWidth > 0 && m_ClientHeight > 0)
		{
			m_RenderingContext->Resize(m_ClientWidth, m_ClientWidth);
			if(m_Scene->GetMainCamera())
				m_Scene->GetMainCamera()->GetCamera().SetViewportDimentions(m_ClientWidth, m_ClientHeight);
		}

	}

	void Application::Update(const GameTimer& timer)
	{
		for (Layer* layer : m_LayerStack)
			layer->OnUpdate(timer);

		m_RenderingContext->OnUpdate();
		m_Scene->UpdateScene(timer);
		if (m_Scene->GetMainCamera())
			Application::Get().GetRenderingContext()->UpdateMainPassConstantBuffers(
				timer, m_Scene->GetMainCamera()->GetCamera()
			);
		Application::Get().GetRenderingContext()->UpdateObjectsConstantBuffers(*m_Scene);
	}

	void Application::Draw(const GameTimer& timer)
	{
		m_RenderingContext->BeginFrame();

		m_RenderingContext->DrawSceneMeshes(*m_Scene);

		m_RenderingContext->EndFrame();
	}

#pragma region Window Event Callbacks
	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
		dispatcher.Dispatch<WindowMovedEvent>([this](WindowMovedEvent& e) { return OnWindowMoved(e); });
		dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimize(e); });

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() > 0 && e.GetHeight() > 0)
		{
			m_ClientWidth = e.GetWidth();
			m_ClientHeight = e.GetHeight();
		}
		if (m_RenderingContext && m_RenderingContext->IsInitialized())
		{
			if (e.GetWParam() == SIZE_MINIMIZED)
			{
				m_bPaused = true;
				m_bMinimized = true;
				m_bMaximized = false;
				m_Timer.Stop();
			}
			else if (e.GetWParam() == SIZE_MAXIMIZED)
			{
				m_bPaused = false;
				m_bMinimized = false;
				m_bMaximized = true;
				m_Timer.Start();
				OnResize();
			}
			else if (e.GetWParam() == SIZE_RESTORED)
			{
				if (m_bMinimized)
				{
					m_bPaused = false;
					m_bMinimized = false;
					OnResize();
				}
				// TODO: when restoring window size from mazimized the application is
				// paused
				else if (m_bMaximized)
				{
					m_bPaused = false;
					m_bMaximized = false;
					OnResize();
				}
				if (m_bResizing)
				{
				}
				else
					OnResize();
			}
		}
		return false;
	}

	bool Application::OnWindowMoved(WindowMovedEvent& e)
	{
		if (e.GetMoveStarted())
		{
			m_bPaused = true;
			m_bResizing = true;
			m_Timer.Stop();
		}
		else
		{
			m_bPaused = false;
			m_bResizing = false;
			m_Timer.Start();
			OnResize();
		}
		return false;
	}

	bool Application::OnWindowMinimize(WindowMinimizeEvent& e)
	{
		if (e.IsMinimized())
		{
			m_bPaused = true;
			m_Timer.Stop();
		}
		else
		{
			m_bPaused = false;
			m_Timer.Start();
		}
		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return false;
	}

#pragma endregion

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::PopOverlay(Layer* overlay)
	{
		m_LayerStack.PopOverlay(overlay);
		overlay->OnDetach();
	}

	void Application::CalculateFrameStats()
	{
		static int frameCnt = 0;
		static float timeElapsed = 0.0f;

		frameCnt++;

		// Compute averages over one second period.
		if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
		{
			auto fps = (float)frameCnt; // fps = frameCnt / 1
			float mspf = 1000.0f / fps;

			std::wstring fpsStr = std::to_wstring(fps);
			std::wstring mspfStr = std::to_wstring(mspf);

			std::wstring wndName(m_AppDescription.Name.begin(), m_AppDescription.Name.end());
			std::wstring windowText = wndName +
				L"    fps: " + fpsStr +
				L"   mspf: " + mspfStr;

			SetWindowText(m_Window->GetNativeWindow(), windowText.c_str());

			// Reset for next average.
			frameCnt = 0;
			timeElapsed += 1.0f;
		}
	}

	float Application::AspectRatio() const
	{
		return float(m_ClientWidth)/m_ClientHeight;
	}

}