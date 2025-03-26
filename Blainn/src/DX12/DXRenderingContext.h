#pragma once

#include "DXFrameResource.h"

#include <dxgi1_4.h>
#include <d3d12.h>
#include <memory>
#include <unordered_set>
#include <wrl.h>
#include "Util/d3dx12.h"

extern const int g_NumFrameResources;
extern const UINT32 g_NumObjects;

namespace dx12lib
{
	class DescriptorAllocator;
	class DescriptorAllocation;
	class RootSignature;
}

namespace Blainn
{
	class Camera;
	class DXDevice;
	class DXMaterial;
	class DXModel;
	class DXResourceManager;
	class DXShader;
	class GameTimer;
	class Scene;
	class StaticMeshComponent;
	class Window;

	class DXRenderingContext
	{
	public:
		DXRenderingContext() = default;
		~DXRenderingContext();

		void Init(std::shared_ptr<Window> wnd);
		void CreateResources(std::shared_ptr<DXResourceManager> resourceManager);

		void BeginFrame();
		void EndFrame();

		void Draw();

		void OnUpdate();
		void UpdateObjectsConstantBuffers();
		void UpdateMaterialsConstantBuffers(std::unordered_set<DXMaterial*> materials);
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
		void BuildRootSignature();
		void BuildPipelineState(); // the legendary PSO
		void BuildFrameResources();

		ID3D12Resource* CurrentBackBuffer() const
		{
			return m_SwapChainBuffer[m_CurrBackBuffer].Get();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;

		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_DXGIFactory;
		std::shared_ptr<DXDevice> m_Device;
		std::shared_ptr<DXResourceManager> m_ResourceManager;

		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		static const int s_SwapChainBufferCount = 2;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[s_SwapChainBufferCount];
		int m_CurrBackBuffer = 0;

		Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
		UINT64 m_CurrentFence;
		HANDLE m_FenceEvent;

		// TODO: should probably create some Pipeline class that would encapsulate
		// the pipeline creation and some pipeline manager to bind pipeline and stuff
		//Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
		std::shared_ptr<dx12lib::RootSignature> m_RootSignature;

		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

		// TODO: this should also be moved into another shader manager(or library, 
		// like in Hazel) class that would manage shaders and give them out based
		// on what the pipeline asks for
		std::unordered_map<std::string, std::shared_ptr<DXShader>> m_Shaders;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;

		Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;

		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CBVHeap;

		std::unique_ptr<dx12lib::DescriptorAllocation> m_RTVAllocation;
		std::unique_ptr<dx12lib::DescriptorAllocation> m_DSVAllocation;
		std::unique_ptr<dx12lib::DescriptorAllocation> m_SRVAllocation;
		std::unique_ptr<dx12lib::DescriptorAllocation> m_CBVAllocation;

		// These frame resources hold per object and per-frame data
		std::vector<std::unique_ptr<DXFrameResource>> m_FrameResources;
		DXFrameResource* m_CurrentFrameResource;
		UINT m_CurrentFrameResourceIndex = 0;
		UINT m_PassConstantBufferOffset;

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
