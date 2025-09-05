#include "pch.h"
#include "EffectPSO.h"

#include "DX12/CascadeShadowMaps.h"

#include "SimpleMath.h"

#include <dx12lib/CommandList.h>
#include <dx12lib/Device.h>
//#include <dx12lib/Helpers.h>
#include <dx12lib/Material.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/VertexTypes.h>

#include <d3d12.h>
#include <d3dcompiler.h>
#include <Util/d3dx12.h>
#include <wrl/client.h>

using namespace Blainn;
using namespace Microsoft::WRL;
using namespace dx12lib;

EffectPSO::EffectPSO(
    std::shared_ptr<dx12lib::Device> device,
    ComPtr<ID3DBlob> vertexShaderBlob,
    ComPtr<ID3DBlob> pixelShaderBlob,
    bool enableLighting,
    bool enableDecal,
    D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType)
    : m_Device(device)
    , m_DirtyFlags(DF_All)
    , m_pPreviousCommandList(nullptr)
    , m_EnableLighting(enableLighting)
    , m_EnableDecal(enableDecal)
    , m_PrimitiveTopologyType(primitiveTopologyType)
{
    //m_pAlignedMVP = (MVP*)_aligned_malloc(sizeof(MVP), 16);

    // Create a root signature.
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // Descriptor range for the textures.
    CD3DX12_DESCRIPTOR_RANGE1 descriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 3);
    CD3DX12_DESCRIPTOR_RANGE1 shadowDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 11);

    // clang-format off
    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
    rootParameters[RootParameters::PerPassDataCB    ].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE);
    rootParameters[RootParameters::MaterialCB       ].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::LightPropertiesCB].InitAsConstants         (sizeof(LightProperties) / 4, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::CascadeDataCB    ].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::PerObjectDataSB  ].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameters::PointLights      ].InitAsShaderResourceView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::SpotLights       ].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::DirectionalLights].InitAsShaderResourceView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::Textures         ].InitAsDescriptorTable   (1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[RootParameters::ShadowMaps       ].InitAsDescriptorTable   (1, &shadowDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(
        RootParameters::NumRootParameters,
        rootParameters,
        staticSamplers.size(), staticSamplers.data(),
        rootSignatureFlags
    );
    // clang-format on

    m_RootSignature = m_Device->CreateRootSignature(rootSignatureDescription.Desc_1_1);

    // Setup the pipeline state.
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
    DXGI_SAMPLE_DESC sampleDesc = m_Device->GetMultisampleQualityLevels(backBufferFormat);

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = backBufferFormat;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
    if (m_EnableDecal) {
        // Disable backface culling on decal geometry.
        rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    }

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.RasterizerState = rasterizerState;
    pipelineStateStream.InputLayout = VertexPositionNormalTangentBitangentTexture::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = primitiveTopologyType;
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

    // Create an SRV that can be used to pad unused texture slots.
    D3D12_SHADER_RESOURCE_VIEW_DESC shadowSRV;
    shadowSRV.Format = DXGI_FORMAT_R32_FLOAT;
    shadowSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    shadowSRV.Texture2D.MostDetailedMip = 0;
    shadowSRV.Texture2D.MipLevels = 1;
    shadowSRV.Texture2D.PlaneSlice = 0;
    shadowSRV.Texture2D.ResourceMinLODClamp = 0;
    shadowSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    m_ShadowMapSRV = m_Device->CreateShaderResourceView(nullptr, &shadowSRV);
}

EffectPSO::~EffectPSO()
{
}

void EffectPSO::Apply(dx12lib::CommandList& commandList)
{
    commandList.SetPipelineState(m_PipelineStateObject);
    commandList.SetGraphicsRootSignature(m_RootSignature);

    if (m_DirtyFlags & DF_PerObjectData)
    {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PerObjectDataSB, m_ObjectData);
    }

    if (m_DirtyFlags & DF_PerPassData)
    {
        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerPassDataCB, m_PassData);
    }

    if (m_DirtyFlags & DF_CascadeData)
    {
        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::CascadeDataCB, m_CascadeData);
    }

    if (m_DirtyFlags & DF_Material)
    {
        if (m_Material)
        {
            const auto& materialProps = m_Material->GetMaterialProperties();

            commandList.SetGraphicsDynamicConstantBuffer(RootParameters::MaterialCB, materialProps);

            using TextureType = Material::TextureType;

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

    if (m_DirtyFlags & DF_ShadowMaps)
    {
        if (m_ShadowMap)
        {
            for (int i = 0; i < CASCADE_COUNT; i++)
            {
                BindTexture(commandList, RootParameters::ShadowMaps, i, m_ShadowMap->GetSlice(CascadeSlice(i)));
            }
        }
    }

    if (m_DirtyFlags & DF_PointLights)
    {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::PointLights, m_PointLights);
    }

    if (m_DirtyFlags & DF_SpotLights)
    {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::SpotLights, m_SpotLights);
    }

    if (m_DirtyFlags & DF_DirectionalLights)
    {
        commandList.SetGraphicsDynamicStructuredBuffer(RootParameters::DirectionalLights, m_DirectionalLights);
    }

    if (m_DirtyFlags & (DF_PointLights | DF_SpotLights | DF_DirectionalLights))
    {
        LightProperties lightProps;
        lightProps.NumPointLights = static_cast<uint32_t>(m_PointLights.size());
        lightProps.NumSpotLights = static_cast<uint32_t>(m_SpotLights.size());
        lightProps.NumDirectionalLights = static_cast<uint32_t>(m_DirectionalLights.size());

        commandList.SetGraphics32BitConstants(RootParameters::LightPropertiesCB, lightProps);
    }

    // Clear the dirty flags to avoid setting any states the next time the effect is applied.
    m_DirtyFlags = DF_None;
}

void EffectPSO::BindTexture(dx12lib::CommandList& commandList, RootParameters rootParameter, uint32_t offset, const std::shared_ptr<dx12lib::Texture>& texture)
{
    if (texture)
    {
        commandList.SetShaderResourceView(rootParameter, offset, texture,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
    else
    {
        commandList.SetShaderResourceView(rootParameter, offset, m_DefaultSRV,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Blainn::EffectPSO::GetStaticSamplers()
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
