#pragma once

#include "DXDevice.h"
#include <wrl.h>


namespace Blainn
{
	class Window;

	class DXRenderingContext 
	{
	public:
		DXRenderingContext() = default;
		~DXRenderingContext();

		void Init(std::shared_ptr<Window> wnd);

		void BeginFrame();
		void EndFrame();

		void Resize(int newWidth, int newHeight);
		void FlushCommandQueue();
		void WaitForGPU();

		bool IsInitialized() const { return m_bIsInitialized; }

		std::shared_ptr<DXDevice> GetDevice() const { return m_Device; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return m_CommandQueue; } 

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return m_CommandList; } 

	private:
		void CreateDepthStencilBuffer(int width, int height);

		ID3D12Resource* CurrentBackBuffer() const
			{ return m_SwapChainBuffer[m_CurrBackBuffer].Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
			{ return m_DsvHeap->GetCPUDescriptorHandleForHeapStart(); }
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(
				m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
				m_CurrBackBuffer,
				m_RtvDescriptorSize
			);
		}

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_DXGIFactory;
		std::shared_ptr<DXDevice> m_Device;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		static const int s_SwapChainBufferCount = 2;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainBufferCount];
		int m_CurrBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		UINT64 m_CurrentFence;
		HANDLE m_FenceEvent;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeap;

		UINT m_RtvDescriptorSize = 0;
		UINT m_DsvDescriptorSize = 0;
		UINT m_CbvSrvUavDescriptorSize = 0;

		D3D12_VIEWPORT m_ScreenViewport;
		D3D12_RECT m_ScissorRect;

		D3D_DRIVER_TYPE m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		bool m_bIsInitialized = false;
	};
}
