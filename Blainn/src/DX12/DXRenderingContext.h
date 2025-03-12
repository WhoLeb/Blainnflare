#pragma once

#include "Core/GameTimer.h"
#include "DXDevice.h"
#include "DXFrameResource.h"
#include "DXShader.h"

#include <dxgi1_4.h>
#include <d3d12.h>
#include <memory>
#include <wrl.h>
#include "Util/d3dx12.h"

extern const int g_NumFrameResources;
extern const UINT32 g_NumObjects;

namespace Blainn
{
	class Window;
	class StaticMeshComponent;
	class DXModel;
	class Scene;
	class Camera;

	class DXRenderingContext 
	{
	public:
		DXRenderingContext() = default;
		~DXRenderingContext();

		void Init(std::shared_ptr<Window> wnd);
		void CreateResources();

		void BeginFrame();
		void EndFrame();

		void DrawSceneMeshes(const Scene& scene);

		void OnUpdate();
		void UpdateObjectsConstantBuffers(const Scene& scene);
		void UpdateMainPassConstantBuffers(
			const GameTimer& gt, const Camera& camera
		);

		void Resize(int newWidth, int newHeight);
		void FlushCommandQueue();
		void WaitForGPU();

		bool IsInitialized() const { return m_bIsInitialized; }

		std::shared_ptr<DXDevice> GetDevice() const { return m_Device; }
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return m_CommandQueue; } 

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return m_CommandList; } 

	private:
		void CreateDepthStencilBuffer(int width, int height);

		void BuildDescriptorHeaps();
		void BuildConstantBufferViews();
		void BuildRootSignature();
		void BuildPipelineState(); // the legendary PSO
		void BuildFrameResources();

		ID3D12Resource* CurrentBackBuffer() const
			{ return m_SwapChainBuffer[m_CurrBackBuffer].Get(); }
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
			{ return m_DSVHeap->GetCPUDescriptorHandleForHeapStart(); }
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const
		{
			return CD3DX12_CPU_DESCRIPTOR_HANDLE(
				m_RTVHeap->GetCPUDescriptorHandleForHeapStart(),
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

		// TODO: should probably create some Pipeline class that would encapsulate
		// the pipeline creation and some pipeline manager to bind pipeline and stuff
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

		// TODO: this should also be moved into another shader manager(or library, 
		// like in Hazel) class that would manage shaders and give them based
		// on what the pipeline asks for
		std::unordered_map<std::string, std::shared_ptr<DXShader>> m_Shaders;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CBVHeap;

		// These frame resources hold per object and per-frame data
		std::vector<std::unique_ptr<DXFrameResource>> m_FrameResources;
		DXFrameResource* m_CurrentFrameResource;
		UINT m_CurrentFrameResourceIndex = 0;
		UINT m_PassConstantBufferOffset;

		PassConstants m_MainPassCB;

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
