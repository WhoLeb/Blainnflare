#pragma once

#include "DXFrameResource.h"

#include <dxgi1_4.h>
#include <d3d12.h>
#include <memory>
#include <unordered_set>
#include <wrl.h>
#include "Util/d3dx12.h"

#include "dx12lib/RenderTarget.h"

extern const int g_NumFrameResources;
extern const UINT32 g_NumObjects;

namespace dx12lib
{
	class Device;
	class DescriptorAllocator;
	class DescriptorAllocation;
	class PipelineStateObject;
	class RenderTarget;
	class RootSignature;
	class SwapChain;
}

namespace Blainn
{
	class Camera;
	class DXDevice;
	class DXMaterial;
	class DXModel;
	class DXResourceManager;
	class DXShader;
	class EffectPSO;
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
		void CreateResources();

		void Draw();

		void OnUpdate();
		void UpdateObjectsConstantBuffers();
		void UpdateMaterialsConstantBuffers(std::unordered_set<DXMaterial*> materials);
		void UpdateMainPassConstantBuffers(
			const GameTimer& gt, const Camera& camera
		);

		void Resize(int newWidth, int newHeight);

		bool IsInitialized() const { return m_bIsInitialized; }

		std::shared_ptr<dx12lib::Device> GetDevice() const { return m_Device; }

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_DXGIFactory;
		//std::shared_ptr<DXDevice> m_Device;
		std::shared_ptr<dx12lib::Device> m_Device;

		std::shared_ptr<dx12lib::SwapChain> m_SwapChain;

		dx12lib::RenderTarget m_RenderTarget;

		// TODO: should probably create some Pipeline class that would encapsulate
		// the pipeline creation and some pipeline manager to bind pipeline and stuff
		std::shared_ptr<dx12lib::RootSignature> m_RootSignature;

		std::unordered_map<std::string, std::shared_ptr<EffectPSO>> m_PSOs;

		// TODO: this should also be moved into another shader manager(or library, 
		// like in Hazel) class that would manage shaders and give them out based
		// on what the pipeline asks for
		// std::unordered_map<std::string, std::shared_ptr<DXShader>> m_Shaders;

		// These frame resources hold per object and per-frame data
		//std::vector<std::unique_ptr<DXFrameResource>> m_FrameResources;
		//DXFrameResource* m_CurrentFrameResource;
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
