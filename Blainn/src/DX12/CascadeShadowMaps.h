#pragma once

#include "EffectPSO.h"

#include <array>
#include <cstdint>
#include <memory>
//#include <pair>
#include <vector>

#include <d3d12.h>

#include "SimpleMath.h"

namespace Blainn
{
	class Camera;
}

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

#define CASCADE_COUNT 4

namespace Blainn
{
	class CascadeShadowMaps
	{
	public:
		CascadeShadowMaps(std::shared_ptr<dx12lib::Device> device, DirectX::XMUINT2 size);

		const std::shared_ptr<dx12lib::Texture>& GetSlice(CascadeSlice slice) const;

		dx12lib::RenderTarget& GetRenderTarget(CascadeSlice slice);
		const dx12lib::RenderTarget& GetRenderTarget(CascadeSlice slice) const;

		DirectX::XMUINT2 GetSize() const;
		D3D12_VIEWPORT GetViewport() const;

		void UpdateCascadeData(DirectX::SimpleMath::Matrix& invViewProj,
			DirectX::SimpleMath::Vector3 lightDirection);
		void UpdateCascadeMatrices(const Blainn::Camera& camera, const DirectX::SimpleMath::Vector3& lightDirection);
		void UpdateCascadeDistances(const std::vector<float>& distances);

		CascadeData& GetCascadeData();

		std::vector<DXGI_FORMAT> GetShadowMapFormats() const;

		void Reset();

	private:
		using ShadowMapList = std::vector<std::shared_ptr<class ShadowMap>>;
		ShadowMapList m_ShadowMaps;

		DirectX::XMUINT2 m_Size;
		D3D12_VIEWPORT m_Viewport;

		CascadeData m_CascadeData;
		DirectX::SimpleMath::Vector3 m_LightDirection;

		std::vector<std::pair<float, float>> m_CascadeViewWindows{ {0.f, 0.15f}, {0.14f, 0.35f}, {0.34f, 0.66f}, {0.65f, 1.1f} };
	};

	class ShadowMapPSO
	{
	public:
		struct alignas(16) PerPassData
		{
			DirectX::SimpleMath::Matrix ViewProj = DirectX::SimpleMath::Matrix::Identity;
		};

		enum RootParameters
		{
			PerObjectDataSB = 0,
			PerPassDataCB = 1,
			NumRootParameters,
		};

		ShadowMapPSO(
			std::shared_ptr<dx12lib::Device> device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);

		void XM_CALLCONV SetWorldMatrices(const std::vector<DirectX::SimpleMath::Matrix>& instanceData)
		{
			m_ObjectData.clear();
			m_ObjectData.resize(instanceData.size());
			for(int32_t i = 0; i < instanceData.size(); ++i)
				m_ObjectData[i].WorldMatrix = instanceData[i];

			m_DirtyFlags |= DF_PerObjectData;
		}
		std::vector<DirectX::SimpleMath::Matrix> GetWorldMatrices() const
		{
			std::vector<DirectX::SimpleMath::Matrix> matrices(m_ObjectData.size());
			for (auto& it : m_ObjectData)
				matrices.push_back(it.WorldMatrix);
			return matrices;
		}
		uint32_t GetInstanceCount() const
		{
			return m_ObjectData.size();
		}

		void SetPerPassData(PerPassData& data)
		{
			m_PassData = data;
			m_DirtyFlags |= DF_PerPassData;
		}
		ShadowMapPSO::PerPassData GetPerPassData()
		{
			return m_PassData;
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

		std::vector<PerObjectData> m_ObjectData;
		ShadowMapPSO::PerPassData m_PassData;

		uint32_t m_DirtyFlags;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType;
	};
}
