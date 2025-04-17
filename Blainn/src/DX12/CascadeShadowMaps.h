#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <d3d12.h>

#include "SimpleMath.h"

namespace dx12lib
{
	class CommandList;
	class Device;
	class PipelineStateObject;
	class RenderTarget;
	class RootSignature;
	class Texture;
}

enum CascadeSlice
{
	Slice0,
	Slice1,
	Slice2,
	Slice3,
	NumSlices
};

namespace Blainn
{
	class CascadeShadowMaps
	{
	public:
		CascadeShadowMaps();
		CascadeShadowMaps(CascadeShadowMaps& copy);

		void AttachShadowMap(CascadeSlice slice, std::shared_ptr<dx12lib::Texture> texture);

		std::shared_ptr<dx12lib::Texture> GetSlice(CascadeSlice slice) const;
		const std::vector<std::shared_ptr<dx12lib::Texture>>& GetSlices() const;

		const dx12lib::RenderTarget& GetRenderTarget(CascadeSlice slice) const;
		dx12lib::RenderTarget& GetRenderTarget(CascadeSlice slice);
		const std::vector<dx12lib::RenderTarget>& GetRenderTargets() const;

		void TransitionTo(std::shared_ptr<dx12lib::CommandList> commandList, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ);

		DirectX::XMUINT2 GetSize(CascadeSlice slice);

		std::vector<DXGI_FORMAT> GetShadowMapFormats() const;

		void Reset();

	private:
		using ShadowMapList = std::vector<std::shared_ptr<dx12lib::Texture>>;
		ShadowMapList m_Slices;
		using RenderTargetList = std::vector<dx12lib::RenderTarget>;
		RenderTargetList m_RenderTargets;

		std::vector<DirectX::XMUINT2> m_Sizes;
	};

	class ShadowMapPSO
	{
	public:
		struct alignas(16) PerObjectData
		{
			DirectX::SimpleMath::Matrix WorldMatrix;
		};

		struct alignas(16) PerPassData
		{
			DirectX::SimpleMath::Matrix View = DirectX::SimpleMath::Matrix::Identity;
			DirectX::SimpleMath::Matrix Proj = DirectX::SimpleMath::Matrix::Identity;
			DirectX::SimpleMath::Matrix ViewProj = DirectX::SimpleMath::Matrix::Identity;
		};

		enum RootParameters
		{
			PerObjectDataCB = 0,
			PerPassDataCB = 1,
			NumRootParameters,
		};

		ShadowMapPSO(
			std::shared_ptr<dx12lib::Device> device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);

		void SetWorldMatrix(DirectX::SimpleMath::Matrix& worldMatrix)
		{
			m_ObjectData.WorldMatrix = worldMatrix;
			m_DirtyFlags |= DF_PerObjectData;
		}
		PerObjectData GetPerObjectData()
		{
			m_ObjectData;
		}

		void SetPerPassData(PerPassData& data)
		{
			m_PassData = data;
			m_DirtyFlags |= DF_PerPassData;
		}
		PerPassData GetPerPassData()
		{
			m_PassData;
		}

		void Apply(dx12lib::CommandList& commandList);

	private:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_PerObjectData = (1 << 0),
			DF_PerPassData = (1 << 1),
			DF_All = DF_PerObjectData
				| DF_PerPassData
		};

		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;

		PerObjectData m_ObjectData;
		PerPassData m_PassData;

		uint32_t m_DirtyFlags;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType;
	};
}
