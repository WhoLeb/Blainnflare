#pragma once

//#include "DXFrameResource.h"

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
	class CascadeShadowMaps;
	class Camera;
	class DXDevice;
	class DXMaterial;
	class DXModel;
	class DXResourceManager;
	class DXShader;
	class EffectPSO;
	class GameTimer;
	class Scene;
	class ShadowMapPSO;
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
		std::shared_ptr<dx12lib::Device> m_Device;

		std::shared_ptr<dx12lib::SwapChain> m_SwapChain;

		dx12lib::RenderTarget m_RenderTarget;
		std::shared_ptr<CascadeShadowMaps> m_CascadeShadowMaps;

		std::shared_ptr<dx12lib::RootSignature> m_RootSignature;

		std::unordered_map<std::string, std::shared_ptr<EffectPSO>> m_PSOs;
		std::shared_ptr<ShadowMapPSO> m_SMPSO;

		D3D12_VIEWPORT m_ScreenViewport;
		D3D12_RECT m_ScissorRect;

		DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D32_FLOAT;

		bool m_bIsInitialized = false;
	};
}
