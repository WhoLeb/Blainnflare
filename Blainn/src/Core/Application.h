#pragma once

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
	protected:
		Application(HINSTANCE hinstance, const ApplicationDesc& description);

	public:
		virtual ~Application();

		static inline Application& Get() { return *s_Instance; }

		static bool Initialize(HINSTANCE hInstance, const ApplicationDesc& description = ApplicationDesc());

		int Run();
		void Close();

		virtual void OnInit() {};
		virtual void OnShutdown() {};
		virtual void OnUpdate() {};

		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		inline HINSTANCE GetAppInstance() const { return m_hInstance; }
		inline Window& GetWindow() const { return *m_Window; }

		HINSTANCE GetNativeInstance() const { return m_hInstance; }

	protected:
		virtual void OnResize();
		virtual void Update();
		virtual void Draw();

		virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
		virtual void OnMouseUp(WPARAM btnState, int x, int y)	{}
		virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

	protected:
		bool InitializeMainWindow(HINSTANCE hInstance, const ApplicationDesc& description);
		bool InitializeD3D();

		void CreateCommandObjects();
		void CreateSwapChain();
		void CreateRtvAndDsvDescriptorHeaps();

		void FlushCommandQueue();

		ID3D12Resource* CurrentBackBuffer() const
			{ return m_SwapChainBuffer[m_CurrBackBuffer].Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
			{ return m_DsvHeap->GetCPUDescriptorHandleForHeapStart(); }
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

		//void CalculateFrameStats();

		void LogAdapters();
		void LogAdapterOutputs(IDXGIAdapter* adapter);
		void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	protected:
		ApplicationDesc m_AppDescription;

		static Application* s_Instance;
		HINSTANCE m_hInstance;

		std::unique_ptr<Window> m_Window;

		float m_lastFrameTime = 0.f;

		Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3dDevice;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		UINT64 m_CurrentFence = 0;
		
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

		static const int s_SwapChainBufferCount = 2;
		int m_CurrBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		D3D12_VIEWPORT m_ScreenViewport;
		D3D12_RECT m_ScissorRect;

		UINT m_RtvDescriptorSize = 0;
		UINT m_DsvDescriptorSize = 0;
		UINT m_CbvSrvUavDescriptorSize = 0;

		D3D_DRIVER_TYPE m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		UINT m_4xMsaaQuality;

		int m_ClientWidth;
		int m_ClientHeight;

		bool m_bPaused = false;
		bool m_bMinimized = false;
		bool m_bMaximized = false;
		bool m_bResizing = false;
		bool m_bFullscreen = false;
	};

}
