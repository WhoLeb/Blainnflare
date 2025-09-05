#pragma once
#include "SimpleMath.h"
#include "ShaderTypes.h"

namespace dx12lib
{
	class CommandList;
	class Device;
	class Material;
	class RootSignature;
	class PipelineStateObject;
	class ShaderResourceView;
	class Texture;
}

namespace Blainn
{
	class GPassPSO
	{
	public:

		enum RootParameters
		{
			PerObjectDataSB		= 0, // StructuredBuffer<PerObjectData> ObjectSB : register(t0);
			PerPassDataCB		= 1, // ConstantBuffer<PerPassData> PassCB : register(b0);
			MaterialCB			= 2, // ConstantBuffer<Material> MaterialCB : register( b1 );
			Textures			= 3,
			// Texture2D AmbientTexture       : register( t1 );
			// Texture2D EmissiveTexture      : register( t2 );
			// Texture2D DiffuseTexture       : register( t3 );
			// Texture2D SpecularTexture      : register( t4 );
			// Texture2D SpecularPowerTexture : register( t5 );
			// Texture2D NormalTexture        : register( t6 );
			// Texture2D BumpTexture          : register( t7 );
			// Texture2D OpacityTexture       : register( t8 );
			
			NumRootParameters
		};

		GPassPSO(std::shared_ptr<dx12lib::Device> device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		virtual ~GPassPSO() {}

		const std::shared_ptr<dx12lib::Material>& GetMaterial() const
		{
			return m_Material;
		}
		void SetMaterial(const std::shared_ptr<dx12lib::Material>& material)
		{
			m_Material = material;
			m_DirtyFlags |= DF_Material;
		}

		std::vector<DirectX::SimpleMath::Matrix>& GetWorldMatrices() const
		{
			std::vector<DirectX::SimpleMath::Matrix> matrices(m_ObjectData.size());
			for (auto& it : m_ObjectData)
				matrices.push_back(it.WorldMatrix);
			return matrices;
		}
		void XM_CALLCONV SetWorldMatrices(const std::vector<DirectX::SimpleMath::Matrix>& instanceData)
		{
			m_ObjectData.clear();
			m_ObjectData.resize(instanceData.size());
			for(int32_t i = 0; i < instanceData.size(); ++i)
				m_ObjectData[i].WorldMatrix = instanceData[i];

			m_DirtyFlags |= DF_PerObjectData;
		}
		uint32_t GetInstanceCount() const
		{
			return m_ObjectData.size();
		}
		PerPassData& GetPerPassData()
		{
			return m_PassData;
		}
		void SetPerPassData(PerPassData& data)
		{
			m_PassData = data;
			m_DirtyFlags |= DF_PerPassData;
		}

		void Apply(dx12lib::CommandList& commandList);
	protected:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_Material = (1 << 0),
			DF_PerObjectData = (1 << 1),
			DF_PerPassData = (1 << 2),
			DF_All = (DF_Material
					| DF_PerObjectData
					| DF_PerPassData)
		};
		
		inline void BindTexture(dx12lib::CommandList& commandList, RootParameters rootParameter,
			uint32_t offset, const std::shared_ptr<dx12lib::Texture>& texture);
		
		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
		
		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;
		
		std::shared_ptr<dx12lib::Material> m_Material;
		
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;
		
		std::vector<PerObjectData> m_ObjectData;
		PerPassData m_PassData;
		
		dx12lib::CommandList* m_pPreviousCommandList;

		// Which properties need to be bound to the
		uint32_t m_DirtyFlags;
		
		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType;
	};
}
