#include "Application.h"

#include "pch.h"

#include "Util/ComboboxSelector.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#include <iostream>


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

		BuildDescriptorHeaps();
		BuildConstantBuffers();
		BuildRootSignature();
		BuildShaders();
		BuildPSO();

		m_bPaused = false;
		OnResize();

		return true;
	}

	int Application::Run()
	{
		MSG msg = { 0 };

		m_Timer.Reset();

		static std::vector<DXGraphicsPrimitive::Vertex> vertices =
		{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::White),	XMFLOAT2(0,0)},
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Black),	XMFLOAT2(0,0)},
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Red),		XMFLOAT2(0,0)},
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Green),	XMFLOAT2(0,0)},
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Blue),		XMFLOAT2(0,0)},
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Yellow),	XMFLOAT2(0,0)},
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Cyan),		XMFLOAT2(0,0)},
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Magenta),	XMFLOAT2(0,0)},
		};

		static std::vector<UINT32> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
			1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		box = std::make_shared<DXGraphicsPrimitive>(m_ResourceManager, vertices, &indices);

		static std::vector<DXGraphicsPrimitive::Vertex> vertices2 =
		{
		{ XMFLOAT3(-1.5f, -1.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::White),	XMFLOAT2(0,0)},
		{ XMFLOAT3(-1.5f, +1.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Black),	XMFLOAT2(0,0)},
		{ XMFLOAT3(-3.5f, +1.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Red),	XMFLOAT2(0,0)},
		{ XMFLOAT3(-3.5f, -1.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(Colors::Green),	XMFLOAT2(0,0)}
		};

		static std::vector<UINT32> indices2 =
		{
			// front face
			0, 1, 2,
			0, 2, 3
		};

		m_Square = std::make_shared<DXGraphicsPrimitive>(m_ResourceManager, vertices2, &indices2);

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
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
			m_RenderingContext->Resize(m_ClientWidth, m_ClientWidth);

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&m_Proj, P);
	}

	void Application::Update(const GameTimer& timer)
	{
		float x = m_Radius * sinf(m_Phi) * cosf(m_Theta);
		float z = m_Radius * sinf(m_Phi) * sinf(m_Theta);
		float y = m_Radius * cosf(m_Phi);

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&m_View, view);

		XMMATRIX world = XMLoadFloat4x4(&m_World);
		XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
		XMMATRIX worldViewProj = world * view * proj;

		// Update the constant buffer with the latest worldViewProj matrix.
		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		m_OjbectCB->CopyData(0, objConstants);


		for (Layer* layer : m_LayerStack)
			layer->OnUpdate();
	}

	void Application::Draw(const GameTimer& timer)
	{
		m_RenderingContext->BeginFrame();
		m_RenderingContext->GetCommandList()->SetPipelineState(m_PSO.Get());

		m_RenderingContext->GetCommandList()->SetGraphicsRootSignature(m_RootSignature.Get());

		ID3D12DescriptorHeap* descriptorHeaps[]{ m_CBVHeap.Get() };
		m_RenderingContext->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(m_CBVHeap->GetGPUDescriptorHandleForHeapStart());
		m_RenderingContext->GetCommandList()->SetGraphicsRootDescriptorTable(0, m_CBVHeap->GetGPUDescriptorHandleForHeapStart());

		box->Bind(m_RenderingContext);
		box->Draw(m_RenderingContext);
		
		m_Square->Bind(m_RenderingContext);
		m_Square->Draw(m_RenderingContext);

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

		dispatcher.Dispatch<MouseButtonDownEvent>([this](MouseButtonDownEvent& e) { return OnMouseDown(e); });
		dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) { return OnMouseUp(e); });
		dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) { return OnMouseMove(e); });

		dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressed(e); });

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		m_ClientWidth = e.GetWidth();
		m_ClientHeight = e.GetHeight();
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
		return true;
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

	bool Application::OnMouseDown(MouseButtonDownEvent& e)
	{
		m_LastMousePos.x = e.GetXOffset();
		m_LastMousePos.y = e.GetYOffset();

		SetCapture(m_Window->GetNativeWindow());
		return false;
	}

	bool Application::OnMouseUp(MouseButtonReleasedEvent& e)
	{
		ReleaseCapture();
		return false;
	}

	bool Application::OnMouseMove(MouseMovedEvent& e)
	{
		int x = e.GetX();
		int y = e.GetY();

		float dx = XMConvertToRadians(0.25 * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25 * static_cast<float>(y - m_LastMousePos.y));

		m_Theta += dx;
		m_Phi += dy;

		m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);

		m_LastMousePos.x = x;
		m_LastMousePos.y = y;

		return false;
	}

	bool Application::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == VK_ESCAPE)
			Close();

#if defined DEBUG || defined _DEBUG
		std::string tmpstr = e.ToString();
		std::wstring dbgMsg = std::wstring(tmpstr.begin(), tmpstr.end());
		std::cout << tmpstr << "\n";
		FlushFileBuffers(GetStdHandle(STD_OUTPUT_HANDLE));
#endif

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
			float fps = (float)frameCnt; // fps = frameCnt / 1
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


	void Application::BuildDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_RenderingContext->GetDevice()->Device()->CreateDescriptorHeap(
			&cbvHeapDesc,
			IID_PPV_ARGS(&m_CBVHeap)
		));
	}

	void Application::BuildConstantBuffers()
	{
		m_OjbectCB = std::make_unique<DXUploadBuffer<ObjectConstants>>(m_RenderingContext, 1, true);

		UINT objCbByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_OjbectCB->GetResource()->GetGPUVirtualAddress();

		int boxCbufIndex = 0;
		cbAddress += boxCbufIndex * objCbByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

		m_RenderingContext->GetDevice()->Device()->CreateConstantBufferView(
			&cbvDesc,
			m_CBVHeap->GetCPUDescriptorHandleForHeapStart()
		);
	}

	void Application::BuildRootSignature()
	{
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig, &errorBlob);

		if (errorBlob)
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		ThrowIfFailed(hr);

		ThrowIfFailed(m_RenderingContext->GetDevice()->Device()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&m_RootSignature)
		));
	}

	void Application::BuildShaders()
	{
		m_VShader = std::make_shared<DXShader>(L"src\\Shaders\\color.hlsl", true, nullptr, "VSmain", "vs_5_0");
		m_PShader = std::make_shared<DXShader>(L"src\\Shaders\\color.hlsl", true, nullptr, "PSmain", "ps_5_0");
	}

	void Application::BuildPSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		auto inputLayout = DXGraphicsPrimitive::Vertex::GetElementLayout();
		psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(m_VShader->GetByteCode()->GetBufferPointer()),
			m_VShader->GetByteCode()->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(m_PShader->GetByteCode()->GetBufferPointer()),
			m_PShader->GetByteCode()->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		ThrowIfFailed(m_RenderingContext->GetDevice()->Device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO)));
	}

	float Application::AspectRatio() const
	{
		return float(m_ClientWidth)/m_ClientHeight;
	}

}