#include "Application.h"

#include "../../pch.h"

using Microsoft::WRL::ComPtr;

namespace Blainn
{
	Application* Application::s_Instance = nullptr;

	Application::Application(HINSTANCE hInstance, const ApplicationDesc& description)
		: m_hInstance(hInstance), m_AppDescription(description)
	{
		s_Instance = this;
		
		if (!InitializeMainWindow(hInstance, description))
			return;

	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
	}

	void Application::Close()
	{
	}

	LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		// when the window is activated or deactivated
		case WM_ACTIVATE:
		{
			return 0;
		}
		// when the user resizes the window
		case WM_SIZE:
		{
			return 0;
		}
		// when the user grabs the resize bars
		case WM_ENTERSIZEMOVE:
		{
			return 0;
		}
		// when the user releases the resize bars. Here everithing is reset based on
		// the new dimensions
		case WM_EXITSIZEMOVE:
		{
			return 0;
		}
		// when the window is destroyed
		case WM_DESTROY:
		{
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
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			// TODO: On mouse up
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			// TODO: On mouse move
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
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
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

}