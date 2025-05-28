#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file EffectPSO.h
  *  @date November 12, 2020
  *  @author Jeremiah van Oosten
  *
  *  @brief Basic lighting effect.
  */

#include "EffectPSO.h"
#include "Scene/Light.h"

#include <DirectXMath.h>
#include <SimpleMath.h>

#include <memory>
#include <vector>

#include <wrl.h>

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
}  // namespace dx12lib

namespace Blainn
{
	class CascadeShadowMaps;

	class EffectPSO
	{
	public:
		// An enum for root signature parameters.
		// I'm not using scoped enums to avoid the explicit cast that would be required
		// to use these as root indices in the root signature.
		enum RootParameters
		{
			// Vertex shader parameter
			PerObjectDataSB = 0,  // StructuredData<PerObjectData> PerObjectSB : register(t0);

			PerPassDataCB = 1,	// ConstantBuffer<PerPassData> PerPassCB : register(b1)

			// Pixel shader parameters
			MaterialCB = 2,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
			LightPropertiesCB = 3,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b2 );

			PointLights = 4,        // StructuredBuffer<PointLight> PointLights : register( t0 );
			SpotLights = 5,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
			DirectionalLights = 6,  // StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 )

			Textures = 7,  // Texture2D AmbientTexture       : register( t3 );
			// Texture2D EmissiveTexture : register( t4 );
			// Texture2D DiffuseTexture : register( t5 );
			// Texture2D SpecularTexture : register( t6 );
			// Texture2D SpecularPowerTexture : register( t7 );
			// Texture2D NormalTexture : register( t8 );
			// Texture2D BumpTexture : register( t9 );
			// Texture2D OpacityTexture : register( t10 );

			CascadeDataCB = 8, 
			ShadowMaps = 9, //Texture2D ShadowMap1           : register(t11);
			//Texture2D ShadowMap2           : register(t12);
			//Texture2D ShadowMap3           : register(t13);
			//Texture2D ShadowMap4           : register(t14);

			NumRootParameters
		};

		EffectPSO(
			std::shared_ptr<dx12lib::Device> device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob,
			bool enableLighting,
			bool enableDecal,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		virtual ~EffectPSO();

		const std::vector<PointLight>& GetPointLights() const
		{
			return m_PointLights;
		}
		void SetPointLights(const std::vector<PointLight>& pointLights)
		{
			m_PointLights = pointLights;
			m_DirtyFlags |= DF_PointLights;
		}

		const std::vector<SpotLight>& GetSpotLights() const
		{
			return m_SpotLights;
		}
		void SetSpotLights(const std::vector<SpotLight>& spotLights)
		{
			m_SpotLights = spotLights;
			m_DirtyFlags |= DF_SpotLights;
		}

		const std::vector<DirectionalLight>& GetDirectionalLights() const
		{
			return m_DirectionalLights;
		}
		void SetDirectionalLights(const std::vector<DirectionalLight>& directionalLights)
		{
			m_DirectionalLights = directionalLights;
			m_DirtyFlags |= DF_DirectionalLights;
		}

		const std::shared_ptr<dx12lib::Material>& GetMaterial() const
		{
			return m_Material;
		}
		void SetMaterial(const std::shared_ptr<dx12lib::Material>& material)
		{
			m_Material = material;
			m_DirtyFlags |= DF_Material;
		}

		const std::shared_ptr<CascadeShadowMaps>& GetShadowMap() const
		{
			return m_ShadowMap;
		}
		void SetShadowMap(const std::shared_ptr<CascadeShadowMaps>& shadowMap)
		{
			m_ShadowMap = shadowMap;
			m_DirtyFlags |= DF_ShadowMaps;
		}

		// Set matrices.
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

		PerPassData& GetPerPassData()
		{
			return m_PassData;
		}
		void SetPerPassData(PerPassData& data)
		{
			m_PassData = data;
			m_DirtyFlags |= DF_PerPassData;
		}

		CascadeData& GetCascadeData()
		{
			return m_CascadeData;
		}
		void SetCascadeData(CascadeData& data)
		{
			m_CascadeData = data;
			m_DirtyFlags |= DF_CascadeData;
		}

		// Apply this effect to the rendering pipeline.
		void Apply(dx12lib::CommandList& commandList);

	private:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_PointLights = (1 << 0),
			DF_SpotLights = (1 << 1),
			DF_DirectionalLights = (1 << 2),
			DF_Material = (1 << 3),
			DF_PerObjectData = (1 << 4),
			DF_PerPassData = (1 << 5),
			DF_CascadeData = (1 << 6),
			DF_ShadowMaps = (1 << 7),
			DF_All = DF_PointLights
				| DF_SpotLights
				| DF_DirectionalLights
				| DF_Material
				| DF_PerObjectData
				| DF_PerPassData
				| DF_CascadeData
				| DF_ShadowMaps
		};

		// Helper function to bind a texture to the rendering pipeline.
		inline void BindTexture(dx12lib::CommandList& commandList, RootParameters rootParameter,
			uint32_t offset, const std::shared_ptr<dx12lib::Texture>& texture);

		inline void BindShadowMap(dx12lib::CommandList& commandList, uint32_t offset,
			const std::shared_ptr<dx12lib::Texture>& texture);

		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;

		std::vector<PointLight>       m_PointLights;
		std::vector<SpotLight>        m_SpotLights;
		std::vector<DirectionalLight> m_DirectionalLights;

		// The material to apply during rendering.
		std::shared_ptr<dx12lib::Material> m_Material;
		std::shared_ptr<CascadeShadowMaps> m_ShadowMap;

		// An SRV used pad unused texture slots.
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;
		std::shared_ptr<dx12lib::ShaderResourceView> m_ShadowMapSRV;

		// Matrices
		std::vector<PerObjectData> m_ObjectData;
		PerPassData m_PassData;
		CascadeData m_CascadeData;

		// If the command list changes, all parameters need to be rebound.
		dx12lib::CommandList* m_pPreviousCommandList;

		// Which properties need to be bound to the
		uint32_t m_DirtyFlags;

		bool m_EnableLighting;
		bool m_EnableDecal;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType;
	};

}
