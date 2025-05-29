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
	class TexturedQuadPSO
	{
	public:

		enum RootParameters
		{
			Texture = 0,
			// Texture2D Texture       : register( t0 );

			NumRootParameters
		};

		TexturedQuadPSO(std::shared_ptr<dx12lib::Device> device,
			Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob);

		void Apply(dx12lib::CommandList& commandList);

		void SetTexture(const std::shared_ptr<dx12lib::Texture>& tex)
		{
			m_Texture = tex;
			m_DirtyFlags = DF_Texture;
		}

	protected:
		enum DirtyFlags
		{
			DF_None = 0,
			DF_Texture = (1 << 0),
			DF_All = DF_Texture
		};

		void BindTexture(dx12lib::CommandList& commandList, RootParameters rootParameter,
			uint32_t offset, const std::shared_ptr<dx12lib::Texture>& texture);

		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

		std::shared_ptr<dx12lib::Device>              m_Device;
		std::shared_ptr<dx12lib::RootSignature>       m_RootSignature;
		std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;

		std::shared_ptr<dx12lib::Texture> m_Texture;
		std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;

		dx12lib::CommandList* m_pPreviousCommandList;

		// Which properties need to be bound to the
		uint32_t m_DirtyFlags;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType;
	};
}
