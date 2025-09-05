#pragma once
#include "ShaderTypes.h"
#include "Scene/Light.h"

namespace Blainn
{
	class CascadeShadowMaps;
	class GBuffer;
}

namespace dx12lib
{
	class ShaderResourceView;
	class Texture;
	class CommandList;
	class PipelineStateObject;
	class RootSignature;
	class Device;
}

namespace Blainn
{
	class DirectLightsPSO
	{
	public:
		enum RootParameters
		{
			PerPassDataCB = 0, // ConstantBuffer<PerPassData> PassCB : register( b0 );
			GBufferTextures = 1, //
			// Texture2D GBuffer_AlbedoOpacity		: register(t0);
			// Texture2D GBuffer_NormalSpecPower	: register(t1);
			// Texture2D GBuffer_Reflectance		: register(t2);
			// Texture2D GBuffer_EmissiveAmbient	: register(t3);
			// Texture2D SceneDepthTexture			: register(t4);
			CascadeDataCB = 2, // ConstantBuffer<CascadeData> CascadeDataCB : register( b1 );
			CascadeShadowMapsTextures = 3, //
			// Texture2D ShadowMap1 : register( t5 );
			// Texture2D ShadowMap2 : register( t6 );
			// Texture2D ShadowMap3 : register( t7 );
			// Texture2D ShadowMap4 : register( t8 );
			DirectionalLightCB = 4, // ConstantBuffer<DirectionalLight> DirectionalLightCB : register( b2 );
			NumRootParameters
		};
	
		DirectLightsPSO(std::shared_ptr<dx12lib::Device>& device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob
			);

		void Apply(dx12lib::CommandList& commandList);

		const PerPassData& GetPassData() const { return m_PassData; }
		PerPassData& GetPassData() { return m_PassData; }
		void SetPassData(const PerPassData& passData)
		{
			m_PassData = passData;
			m_DirtyFlags |= DF_PassData;
		}
	
		CascadeData& GetCascadeData()
		{
			return m_CascadeData;
		}
		void SetCascadeData(CascadeData& data)
		{
			m_CascadeData = data;
			m_DirtyFlags |= DF_ShadowMaps;
		}

		void SetShadowMap(const std::shared_ptr<CascadeShadowMaps>& shadowMaps)
		{
			m_ShadowMaps = shadowMaps;
			m_DirtyFlags |= DF_ShadowMaps;
		}
		
		void SetGBuffer(const std::shared_ptr<GBuffer>& gBuffer)
		{
			m_GBuffer = gBuffer;
		}
	
		const DirectionalLight& GetDirectionalLights() const
		{
			return m_DirectionalLight;
		}
		void SetDirectionalLight(const DirectionalLight& directionalLight)
		{
			m_DirectionalLight = directionalLight;
			m_DirtyFlags |= DF_LightData;
		}
	
	protected:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_PassData = (1 << 0),
			DF_LightData = (1 << 1),
			DF_ShadowMaps = (1 << 2),
			DF_All	= DF_PassData
					| DF_LightData
					| DF_ShadowMaps
		};
		uint32_t m_DirtyFlags;
	
		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;
	
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultShadowSRV;

		std::shared_ptr<Blainn::CascadeShadowMaps>    m_ShadowMaps;

		PerPassData m_PassData;
		DirectionalLight m_DirectionalLight;
		CascadeData m_CascadeData;
		std::shared_ptr<Blainn::GBuffer> m_GBuffer;

		dx12lib::CommandList* m_PreviousCommandList;
	};

	class PointLightsPSO
	{
	public:
		enum RootParameters
		{
			PerPassDataCB = 0, // ConstantBuffer<PerPassData> PassCB : register( b0 );
			GBufferTextures = 1, // 
			// Texture2D GBuffer_AlbedoOpacity		: register(t0);
			// Texture2D GBuffer_NormalSpecPower	: register(t1);
			// Texture2D GBuffer_Reflectance		: register(t2);
			// Texture2D GBuffer_EmissiveAmbient	: register(t3);
			// Texture2D SceneDepthTexture			: register(t4);
			LightVolumeCB = 2, // ConstantBuffer<PerInstanceData> PerInstanceCB : register(b1, space0);
			PointLightCB = 3,  // ConstantBuffer<PointLight> PointLightCB : register( b1, space1 );
			NumRootParameters
		};
	
		PointLightsPSO(std::shared_ptr<dx12lib::Device>& device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShader, Microsoft::WRL::ComPtr<ID3DBlob> pixelShader);

		void Apply(dx12lib::CommandList& commandList);

		const PerPassData& GetPassData() const { return m_PassData; }
		void SetPassData(const PerPassData& passData)
		{
			m_PassData = passData;
			m_DirtyFlags |= DF_PassData;
		}

		const PointLight& GetLightData() const { return m_LightData; }
		void SetLightData(const PointLight& lightData)
		{
			m_LightData = lightData;
			m_DirtyFlags |= DF_LightData;
		}

		const PerObjectData& GetObjectData() const {return m_ObjectData;}
		void SetObjectData(const PerObjectData& objectData)
		{
			m_ObjectData = objectData;
			m_DirtyFlags |= DF_ObjectData;
		}

		void SetGBuffer(const std::shared_ptr<GBuffer>& gBuffer)
		{
			m_GBuffer = gBuffer;
		}
		
	protected:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_PassData = (1 << 0),
			DF_LightData = (1 << 1),
			DF_ObjectData = (1 << 1),
			DF_All	= DF_PassData
					| DF_LightData
					| DF_ObjectData
		};
		uint32_t m_DirtyFlags;
	
		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;
		
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;
		std::shared_ptr<Blainn::GBuffer> m_GBuffer;

		PerObjectData m_ObjectData;
		PerPassData m_PassData;
		PointLight m_LightData;
	};

	class SpotLightsPSO
	{
	public:
		enum RootParameters
		{
			PerPassDataCB = 0, // ConstantBuffer<PerPassData> PassCB : register( b0 );
			GBufferTextures = 1,
			// Texture2D GBuffer_AlbedoOpacity		: register(t0);
			// Texture2D GBuffer_NormalSpecPower	: register(t1);
			// Texture2D GBuffer_Reflectance		: register(t2);
			// Texture2D GBuffer_EmissiveAmbient	: register(t3);
			// Texture2D SceneDepthTexture			: register(t4);
			LightVolumeCB = 2, // ConstantBuffer<PerInstanceData> PerInstanceCB : register(b1, space0);
			SpotLightCB = 3, // ConstantBuffer<SpotLight> SpotLightCB : register( b1, space1 );
			NumRootParameters
		};
	
		SpotLightsPSO(std::shared_ptr<dx12lib::Device>& device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShader, Microsoft::WRL::ComPtr<ID3DBlob> pixelShader);

		void Apply(dx12lib::CommandList& commandList);
	protected:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_PassData = (1 << 0),
			DF_LightData = (1 << 1),
			DF_All	= DF_PassData
					| DF_LightData
		};
		uint32_t m_DirtyFlags;
	
		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;
		
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;
	};
}