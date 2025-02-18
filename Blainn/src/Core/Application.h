#pragma once

#include "DX12/DXRenderingContext.h"
#include "DX12/DXResourceManager.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "GameTimer.h"
#include "LayerStack.h"
#include "Window.h"

#include "../Util/Util.h"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Blainn
{

	struct ApplicationDesc
	{
		std::string Name = "Blainnflare";
		uint32_t WindowWidth = 1600, WindowHeight = 800;
		bool WindowDecorated = false;
		bool Fullscreen = false;
		bool VSync = true;
	};

	class Application
	{
	public:
		Application(HINSTANCE hinstance, const ApplicationDesc& description = ApplicationDesc());
		virtual ~Application();

		static inline Application& Get() { return *s_Instance; }

		bool Initialize();

		int Run();
		void Close();

		virtual void OnInit() {};
		virtual void OnShutdown() {};
		virtual void OnUpdate() {};

		virtual void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		inline HINSTANCE GetAppInstance() const { return m_hInstance; }
		inline Window& GetWindow() const { return *m_Window; }

		HINSTANCE GetNativeInstance() const { return m_hInstance; }

	protected:
		virtual void OnResize();
		virtual void Update(const GameTimer& timer);
		virtual void Draw(const GameTimer& timer);

		// Window events
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowMoved(WindowMovedEvent& e);
		bool OnWindowMinimize(WindowMinimizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);

		virtual bool OnMouseDown(MouseButtonDownEvent& e)
		{
			std::string text = e.ToString();
			std::wstring wText = std::wstring(text.begin(), text.end());
			MessageBox(nullptr, wText.c_str(), L"Mouse pressed", MB_OK);
			return false;
		}
		virtual bool OnMouseUp(MouseButtonReleasedEvent& e) { return false; }
		virtual bool OnMouseMove(MouseButtonDownEvent& e) { return false; }

		bool OnKeyPressed(KeyPressedEvent& e);

	protected:
		void CalculateFrameStats();

	protected:
		ApplicationDesc m_AppDescription;

		HINSTANCE m_hInstance;

		static Application* s_Instance;

		std::shared_ptr<Window> m_Window;
		std::shared_ptr<DXRenderingContext> m_RenderingContext;
		std::shared_ptr<DXResourceManager> m_ResourceManager;

		GameTimer m_Timer;

		float m_lastFrameTime = 0.f;

		LayerStack m_LayerStack;

		int m_ClientWidth;
		int m_ClientHeight;

		bool m_bPaused = false;
		bool m_bMinimized = false;
		bool m_bMaximized = false;
		bool m_bResizing = false;
		bool m_bFullscreen = false;
	};

}
