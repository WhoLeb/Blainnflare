#include "Application.h"

#include "../../pch.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Blainn
{
	Application* Application::s_Instance = nullptr;

	Application::Application(HINSTANCE hInstance, const ApplicationDesc& description)
		: m_hInstance(hInstance)
		, m_AppDescription(description)
		, m_ClientWidth(description.WindowWidth)
		, m_ClientHeight(description.WindowHeight)
	{
	}

	Application::~Application()
	{
	}

	bool Application::Initialize(HINSTANCE hInstance, const ApplicationDesc& description)
	{
		if (s_Instance)
			return false;

		s_Instance = new Application(hInstance, description);
		
		if (!s_Instance->InitializeMainWindow(hInstance, description))
			return false;

		if (!s_Instance->InitializeD3D())
			return false;

		s_Instance->OnResize();

		return true;
	}

	int Application::Run()
	{
		MSG msg = { 0 };

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if (!m_bPaused)
				{
					Update();
					Draw();
				}
				else
					Sleep(100);
			}
		}
		return (int)msg.wParam;
	}

	void Application::Close()
	{
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

	void Application::Update()
	{
	}

	void Application::Draw()
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

	bool Application::InitializeMainWindow(HINSTANCE hInstance, const ApplicationDesc& description)
	{
		WindowDesc windowDesc = {};
		windowDesc.Title = description.Name;
		windowDesc.Width = description.WindowWidth;
		windowDesc.Height = description.WindowHeight;
		windowDesc.Decorated = description.WindowDecorated;
		windowDesc.Fullscreen = description.Fullscreen;
		windowDesc.VSync = description.VSync;

		m_Window = std::unique_ptr<Window>(Window::Create(hInstance, windowDesc));
		return m_Window->Init();
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

	LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		// when the window is activated or deactivated
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_bPaused = true;
			}
			else
			{
				m_bPaused = false;
			}
			return 0;
		}
		// when the user resizes the window
		case WM_SIZE:
		{
			m_ClientWidth = LOWORD(lParam);
			m_ClientHeight = HIWORD(lParam);
			if (m_D3dDevice)
			{
				if (wParam == SIZE_MINIMIZED)
				{
					m_bPaused = true;
					m_bMinimized = true;
					m_bMaximized = false;
				}
				else if (wParam == SIZE_MAXIMIZED)
				{
					m_bPaused = false;
					m_bMinimized = false;
					m_bMaximized = true;
					OnResize();
				}
				else if (wParam == SIZE_RESTORED)
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
					else if (m_bResizing)
					{
					}
					else
					{
						OnResize();
					}
				}
			}
			return 0;
		}
		// when the user grabs the resize bars
		case WM_ENTERSIZEMOVE:
		{
			m_bPaused = true;
			m_bResizing = true;
			return 0;
		}
		// when the user releases the resize bars. Here everithing is reset based on
		// the new dimensions
		case WM_EXITSIZEMOVE:
		{
			m_bPaused = false;
			m_bResizing = false;
			OnResize();
			return 0;
		}
		// when the window is destroyed
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		// When a user presses a key not corresponding to a mnemonic or accelerator key.
		case WM_MENUCHAR:
			// Don't beep on Alt+Enter
			return MAKELRESULT(0, MNC_CLOSE);

		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y= 200;
			return 0;
		}
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			// TODO: On mouse down
			OnMouseDown(wParam, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			// TODO: On mouse up
			OnMouseUp(wParam, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			// TODO: On mouse move
			OnMouseMove(wParam, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
			return 0;
		}
		case WM_KEYUP:
			if (wParam == VK_ESCAPE)
				PostQuitMessage(0);
			else if ((int)wParam == VK_F2)
				// TODO: Set 4xMsaaState
				return 0;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}


}