#pragma once

#include "DXDevice.h"
#include <wrl.h>
#include "Renderer/RenderContext.h"


namespace Blainn
{
	class Window;

	class DXContext : public RenderContext 
	{
	public:
		DXContext() = default;
		DXContext(IDXGIAdapter1* adapter) {};
		virtual ~DXContext() {};

		virtual void Init(Window wnd) override;

		Microsoft::WRL::ComPtr<IDXGIFactory4> GetFactory() const { return m_DXGIFactory; }
		Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return m_D3DDevice; }
		Microsoft::WRL::ComPtr<IDXGISwapChain> GetSwapChain() const { return m_SwapChain; }

		Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() const { return m_Fence; }
		UINT64 GetCurrentFence() const { return m_CurrentFence; }

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return m_CommandQueue; }
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return m_DirectCmdListAlloc; }
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return m_CommandList; }

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_DXGIFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3DDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		static const int s_SwapChainBufferCount = 2;
		int m_CurrBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		UINT64 m_CurrentFence;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainBufferCount];
		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

		D3D12_VIEWPORT m_ScreenViewport;
		D3D12_RECT m_ScissorRect;

		D3D_DRIVER_TYPE m_D3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
		DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	};
}
