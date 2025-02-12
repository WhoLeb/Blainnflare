#include "Application.h"

#include "pch.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

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


		WindowDesc windowDesc = {};
		windowDesc.Title = m_AppDescription.Name;
		windowDesc.Width = m_AppDescription.WindowWidth;
		windowDesc.Height = m_AppDescription.WindowHeight;
		windowDesc.Decorated = m_AppDescription.WindowDecorated;
		windowDesc.Fullscreen = m_AppDescription.Fullscreen;
		windowDesc.VSync = m_AppDescription.VSync;

		m_Window = std::unique_ptr<Window>(Window::Create(m_hInstance, windowDesc));
		m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
		m_Window->Init();

		if (!InitializeD3D())
			return;

		s_Instance = this;

		OnResize();
	}

	Application::~Application()
	{
	}

	bool Application::Initialize()
	{
		

		return true;
	}

	int Application::Run()
	{
		MSG msg = { 0 };

		m_Timer.Reset();

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
					Sleep(100);
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
		assert(m_D3dDevice);
		assert(m_SwapChain);
		assert(m_DirectCmdListAlloc);

		FlushCommandQueue();

		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

		for (size_t i = 0; i < s_SwapChainBufferCount; ++i)
			m_SwapChainBuffer[i].Reset();
		m_DepthStencilBuffer.Reset();

		ThrowIfFailed(m_SwapChain->ResizeBuffers(
			s_SwapChainBufferCount,
			m_ClientWidth, m_ClientHeight,
			m_BackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		));

		m_CurrBackBuffer = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (size_t i = 0; i < s_SwapChainBufferCount; ++i)
		{
			ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
			m_D3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
			rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
		}

		D3D12_RESOURCE_DESC dsDesc;
		dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		dsDesc.Alignment = 0;
		dsDesc.Width = m_ClientWidth;
		dsDesc.Height = m_ClientHeight;
		dsDesc.DepthOrArraySize = 1;
		dsDesc.MipLevels = 1;

		dsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

		dsDesc.SampleDesc.Count = 1;
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = m_DepthStencilFormat;
		optClear.DepthStencil.Depth = 1.f;
		optClear.DepthStencil.Stencil = 0;
		ThrowIfFailed(m_D3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&dsDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = m_DepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		m_D3dDevice->CreateDepthStencilView(
			m_DepthStencilBuffer.Get(),
			&dsvDesc,
			DepthStencilView()
		);

		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		));

		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();

		m_ScreenViewport.TopLeftX = 0;
		m_ScreenViewport.TopLeftY = 0;
		m_ScreenViewport.Width = (float)m_ClientWidth;
		m_ScreenViewport.Height = (float)m_ClientHeight;
		m_ScreenViewport.MinDepth = 0.f;
		m_ScreenViewport.MaxDepth = 1.f;

		m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };
	}

	void Application::Update(const GameTimer& timer)
	{
	}

	void Application::Draw(const GameTimer& timer)
	{
		ThrowIfFailed(m_DirectCmdListAlloc->Reset());
		
		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		m_CommandList->RSSetViewports(1, &m_ScreenViewport);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::Red, 0, nullptr);
		m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
		
		m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

		ThrowIfFailed(m_CommandList->Close());

		ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		ThrowIfFailed(m_SwapChain->Present(0, 0));

		m_CurrBackBuffer = (m_CurrBackBuffer + 1) % s_SwapChainBufferCount;

		FlushCommandQueue();
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		m_ClientWidth = e.GetWidth();
		m_ClientHeight = e.GetHeight();
		if (m_D3dDevice)
		{
			if (e.GetWParam() == SIZE_MINIMIZED)
			{
				m_bPaused = true;
				m_bMinimized = true;
				m_bMaximized = false;
			}
			else if (e.GetWParam() == SIZE_MAXIMIZED)
			{
				m_bPaused = false;
				m_bMinimized = false;
				m_bMaximized = true;
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

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
		dispatcher.Dispatch<WindowMovedEvent>([this](WindowMovedEvent& e) { return OnWindowMoved(e); });
		dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimize(e); });

		dispatcher.Dispatch<MouseButtonDownEvent>([this](MouseButtonDownEvent& e) { return OnMouseDown(e); });
	}

	void Application::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == VK_ESCAPE)
			Close();
	}

	bool Application::InitializeMainWindow()
	{
		return true;
	}

	bool Application::InitializeD3D()
	{
#if defined (DEBUG) || defined (_DEBUG)
		{
			ComPtr<ID3D12Debug> debugController;
			ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
			debugController->EnableDebugLayer();
		}
#endif

		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DxgiFactory)));

		HRESULT hardwareResult = D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&m_D3dDevice)
		);

		if (FAILED(hardwareResult))
		{
			ComPtr<IDXGIAdapter> pWarpAdapter;

			ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
			
			ThrowIfFailed(D3D12CreateDevice(
				pWarpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(&m_D3dDevice)
			));
		}

		ThrowIfFailed(m_D3dDevice->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_Fence)
		));

		m_RtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_DsvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvUavDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels = {};
		msQualityLevels.Format = m_BackBufferFormat;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;
		ThrowIfFailed(m_D3dDevice->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevels,
			sizeof(msQualityLevels)
		));

		m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level");

#ifdef _DEBUG
		LogAdapters();
#endif

		CreateCommandObjects();
		CreateSwapChain();
		CreateRtvAndDsvDescriptorHeaps();

		return true;
	}

	void Application::CreateCommandObjects()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		ThrowIfFailed(m_D3dDevice->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_CommandQueue)
		));

		ThrowIfFailed(m_D3dDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_DirectCmdListAlloc)
		));

		ThrowIfFailed(m_D3dDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_DirectCmdListAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(&m_CommandList)
		));

		m_CommandList->Close();
	}

	void Application::CreateSwapChain()
	{
		m_SwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = m_AppDescription.WindowWidth;
		sd.BufferDesc.Height = m_AppDescription.WindowHeight;
		sd.BufferDesc.RefreshRate.Numerator = 144;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = m_BackBufferFormat;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = s_SwapChainBufferCount;
		sd.OutputWindow = m_Window->GetNativeWindow();
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		ThrowIfFailed(m_DxgiFactory->CreateSwapChain(
			m_CommandQueue.Get(),
			&sd,
			m_SwapChain.GetAddressOf()
		));
	}

	void Application::CreateRtvAndDsvDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = s_SwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(
			&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));


		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(
			&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
	}

	void Application::FlushCommandQueue()
	{
		m_CurrentFence++;

		ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

		if (m_Fence->GetCompletedValue() < m_CurrentFence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

			ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Application::CurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
			m_CurrBackBuffer,
			m_RtvDescriptorSize
		);
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

	void Application::LogAdapters()
	{
		UINT i = 0;

		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;

		while (m_DxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L"\n";

			OutputDebugString(text.c_str());

			adapterList.push_back(adapter);
			++i;
		}

		for (auto& adapter : adapterList)
		{
			LogAdapterOutputs(adapter);
			ReleaseCom(adapter);
		}
	}

	void Application::LogAdapterOutputs(IDXGIAdapter* adapter)
	{
		UINT i = 0; 
		IDXGIOutput* output = nullptr;

		while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_OUTPUT_DESC desc;
			output->GetDesc(&desc);

			std::wstring text = L"***Output: ";
			text += desc.DeviceName;
			text += L"\n";
			
			OutputDebugString(text.c_str());

			LogOutputDisplayModes(output, m_BackBufferFormat);

			ReleaseCom(output);

			++i;
		}
	}

	void Application::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
	{
		UINT count = 0;
		UINT flags = 0;

		output->GetDisplayModeList(format, flags, &count, nullptr);

		std::vector<DXGI_MODE_DESC> modeList(count);
		output->GetDisplayModeList(format, flags, &count, modeList.data());

		for (auto& x : modeList)
		{
			UINT n = x.RefreshRate.Numerator;
			UINT d = x.RefreshRate.Denominator;
			
			std::wstring text =
				L"Width = " + std::to_wstring(x.Width) + L" " +
				L"Height = " + std::to_wstring(x.Height) + L" " +
				L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
				L"\n";

			OutputDebugString(text.c_str());
		}

	}



}