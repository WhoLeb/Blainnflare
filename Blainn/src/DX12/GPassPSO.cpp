#include "pch.h"
#include "GPassPSO.h"

#include "dx12lib/CommandList.h"
#include "dx12lib/Device.h"
#include "dx12lib/Material.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/VertexTypes.h"

Blainn::GPassPSO::GPassPSO(std::shared_ptr<dx12lib::Device> device, Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob,
							Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType)
								: m_Device(device)
{
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// Descriptor range for the textures.
	CD3DX12_DESCRIPTOR_RANGE1 descriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 1);
	
	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
	rootParameters[RootParameters::PerPassDataCB    ].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
	rootParameters[RootParameters::MaterialCB       ].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::PerObjectDataSB  ].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootParameters::Textures         ].InitAsDescriptorTable   (1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(
		RootParameters::NumRootParameters, rootParameters,
		staticSamplers.size(), staticSamplers.data(),
		rootSignatureFlags
		);

	m_RootSignature = m_Device->CreateRootSignature(rootSignatureDesc.Desc_1_1);
	
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
	} pipelineStateStream;

	// Create a color buffer with sRGB for gamma correction.
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

	// Check the best multisample quality level that can be used for the given back buffer format.
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };
	
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 4;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // AlbedoOpacity
	rtvFormats.RTFormats[1] = DXGI_FORMAT_R10G10B10A2_UNORM; // Normal spec
	rtvFormats.RTFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM; // Reflectance
	rtvFormats.RTFormats[3] = DXGI_FORMAT_R10G10B10A2_UNORM; // EmissiveAmbient

	CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);

	pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.RasterizerState = rasterizerState;
	pipelineStateStream.InputLayout = dx12lib::VertexPositionNormalTangentBitangentTexture::InputLayout;
	pipelineStateStream.PrimitiveTopologyType = primitiveType;
	pipelineStateStream.DSVFormat = depthBufferFormat;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.SampleDesc = sampleDesc;

	m_PipelineStateObject = m_Device->CreatePipelineStateObject(pipelineStateStream);

	// Create an SRV that can be used to pad unused texture slots.
	D3D12_SHADER_RESOURCE_VIEW_DESC defaultSRV;
	defaultSRV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	defaultSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	defaultSRV.Texture2D.MostDetailedMip = 0;
	defaultSRV.Texture2D.MipLevels = 1;
	defaultSRV.Texture2D.PlaneSlice = 0;
	defaultSRV.Texture2D.ResourceMinLODClamp = 0;
	defaultSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_DefaultSRV = m_Device->CreateShaderResourceView(nullptr, &defaultSRV);
}

void Blainn::GPassPSO::Apply(dx12lib::CommandList& commandList)
{
	commandList.SetPipelineState(m_PipelineStateObject);
	commandList.SetGraphicsRootSignature(m_RootSignature);

	if (m_DirtyFlags & DF_PerObjectData)
		commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PerObjectDataSB, m_ObjectData);

	if (m_DirtyFlags & DF_PerPassData)
		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerPassDataCB, m_PassData);

	if (m_DirtyFlags & DF_Material)
	{
		if (m_Material)
		{
			const auto& materialProps = m_Material->GetMaterialProperties();

			commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, materialProps);

			using TextureType = dx12lib::Material::TextureType;

			BindTexture(commandList, RootParameters::Textures, 0, m_Material->GetTexture(TextureType::Ambient));
			BindTexture(commandList, RootParameters::Textures, 1, m_Material->GetTexture(TextureType::Emissive));
			BindTexture(commandList, RootParameters::Textures, 2, m_Material->GetTexture(TextureType::Diffuse));
			BindTexture(commandList, RootParameters::Textures, 3, m_Material->GetTexture(TextureType::Specular));
			BindTexture(commandList, RootParameters::Textures, 4, m_Material->GetTexture(TextureType::SpecularPower));
			BindTexture(commandList, RootParameters::Textures, 5, m_Material->GetTexture(TextureType::Normal));
			BindTexture(commandList, RootParameters::Textures, 6, m_Material->GetTexture(TextureType::Bump));
			BindTexture(commandList, RootParameters::Textures, 7, m_Material->GetTexture(TextureType::Opacity));
		}
	}
}

void Blainn::GPassPSO::BindTexture(dx12lib::CommandList& commandList, RootParameters rootParameter, uint32_t offset,
	const std::shared_ptr<dx12lib::Texture>& texture)
{
	if (texture)
		commandList.SetShaderResourceView(rootParameter, offset, texture,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	else
		commandList.SetShaderResourceView(rootParameter, offset, m_DefaultSRV,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Blainn::GPassPSO::GetStaticSamplers()
{
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.f,
		8
	);
	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.f,
		8
	);
	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp
	};
}
