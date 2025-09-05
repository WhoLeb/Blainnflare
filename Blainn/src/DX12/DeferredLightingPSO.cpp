#include "pch.h"
#include "DeferredLightingPSO.h"

#include "CascadeShadowMaps.h"
#include "GBuffer.h"
#include "VertexTypes.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/Device.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/VertexTypes.h"

using namespace Blainn;

static void BindTexture(dx12lib::CommandList& commandList, int32_t rootParameter, uint32_t offset,
		const std::shared_ptr<dx12lib::Texture>& texture, std::shared_ptr<dx12lib::ShaderResourceView>& defaultSRV)
{
	if (texture)
	{
		commandList.SetShaderResourceView(rootParameter, offset, texture,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	else
	{
		commandList.SetShaderResourceView(rootParameter, offset, defaultSRV,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
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

DirectLightsPSO::DirectLightsPSO(std::shared_ptr<dx12lib::Device>& device,
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob)
		: m_Device(device)
		, m_DirtyFlags(DF_All)
		, m_PreviousCommandList(nullptr)
{
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	
	CD3DX12_DESCRIPTOR_RANGE1 gBufferDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);
	CD3DX12_DESCRIPTOR_RANGE1 shadowDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 5);

	// clang-format off
	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
	rootParameters[RootParameters::PerPassDataCB			].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::CascadeDataCB			].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::DirectionalLightCB		].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::GBufferTextures			].InitAsDescriptorTable   (1, &gBufferDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::CascadeShadowMapsTextures].InitAsDescriptorTable   (1, &shadowDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(
		RootParameters::NumRootParameters,
		rootParameters,
		staticSamplers.size(), staticSamplers.data(),
		rootSignatureFlags
	);
	
    m_RootSignature = m_Device->CreateRootSignature(rootSignatureDescription.Desc_1_1);
	
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
		CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendDesc;
		CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencil;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSFormat;
		CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
	} pipelineStateStream;

	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.LogicOpEnable = false;          // Disable logic operations (typical for blending)
	rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;     // Source color factor
	rtBlendDesc.DestBlend = D3D12_BLEND_ONE;    // Destination color factor
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;   // Add source and destination for color
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;     // Source alpha factor
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;    // Destination alpha factor
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;   // Add source and destination for alpha
	rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;       // No-op since LogicOpEnable is false
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // Write to all channels (RGBA)
	
	CD3DX12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.RenderTarget[0] = rtBlendDesc;
	
	// Create a color buffer with sRGB for gamma correction.
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    // Check the best multisample quality level that can be used for the given back buffer format.
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = backBufferFormat;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	
    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.BlendDesc = blendDesc;
    pipelineStateStream.RasterizerState = rasterizerState;
	pipelineStateStream.DepthStencil = depthStencilDesc;
    pipelineStateStream.InputLayout = dx12lib::VertexPosition::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.DSFormat = depthBufferFormat;
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
	
	D3D12_SHADER_RESOURCE_VIEW_DESC defaultShadowSRV;
	defaultShadowSRV.Format = DXGI_FORMAT_R32_FLOAT;
	defaultShadowSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	defaultShadowSRV.Texture2D.MostDetailedMip = 0;
	defaultShadowSRV.Texture2D.MipLevels = 1;
	defaultShadowSRV.Texture2D.PlaneSlice = 0;
	defaultShadowSRV.Texture2D.ResourceMinLODClamp = 0;
	defaultShadowSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_DefaultShadowSRV = m_Device->CreateShaderResourceView(nullptr, &defaultShadowSRV);
}

void DirectLightsPSO::Apply(dx12lib::CommandList& commandList)
{
	commandList.SetPipelineState(m_PipelineStateObject);
	commandList.SetGraphicsRootSignature(m_RootSignature);
	commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	if (m_DirtyFlags & DF_PassData)
		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerPassDataCB, m_PassData);

	if (m_DirtyFlags & DF_ShadowMaps)
		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::CascadeDataCB, m_CascadeData);

	if (m_DirtyFlags & DF_LightData)
		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::DirectionalLightCB, m_DirectionalLight);

	if (m_GBuffer)
	{
		using TextureType = GBuffer::TextureType;
		BindTexture(commandList, RootParameters::GBufferTextures, 0, m_GBuffer->GetTexture(TextureType::AlbedoOpacity), m_DefaultSRV);
		BindTexture(commandList, RootParameters::GBufferTextures, 1, m_GBuffer->GetTexture(TextureType::NormalSpec), m_DefaultSRV);
		BindTexture(commandList, RootParameters::GBufferTextures, 2, m_GBuffer->GetTexture(TextureType::Reflectance), m_DefaultSRV);
		BindTexture(commandList, RootParameters::GBufferTextures, 3, m_GBuffer->GetTexture(TextureType::EmissiveAmbient), m_DefaultSRV);
		BindTexture(commandList, RootParameters::GBufferTextures, 4, m_GBuffer->GetTexture(TextureType::Depth), m_DefaultSRV);
	}
	else
		printf("[DirectLightsPSO::Apply] Error! No GBuffer set!!!\n");

	if (m_DirtyFlags & DF_ShadowMaps)
	{
		if (m_ShadowMaps)
		{
			BindTexture(commandList, RootParameters::CascadeShadowMapsTextures, 0, m_ShadowMaps->GetSlice(CascadeSlice::Slice0), m_DefaultShadowSRV);
			BindTexture(commandList, RootParameters::CascadeShadowMapsTextures, 1, m_ShadowMaps->GetSlice(CascadeSlice::Slice1), m_DefaultShadowSRV);
			BindTexture(commandList, RootParameters::CascadeShadowMapsTextures, 2, m_ShadowMaps->GetSlice(CascadeSlice::Slice2), m_DefaultShadowSRV);
			BindTexture(commandList, RootParameters::CascadeShadowMapsTextures, 3, m_ShadowMaps->GetSlice(CascadeSlice::Slice3), m_DefaultShadowSRV);
		}
	}

	m_DirtyFlags = DF_None;
}

PointLightsPSO::PointLightsPSO(std::shared_ptr<dx12lib::Device>& device,
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader,
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader)
    : m_Device(device)
    , m_DirtyFlags(DF_All)
{
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE1 gBufferDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);

    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
    rootParameters[RootParameters::PerPassDataCB	].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[RootParameters::LightVolumeCB	].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[RootParameters::PointLightCB		].InitAsConstantBufferView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootParameters::GBufferTextures	].InitAsDescriptorTable(1, &gBufferDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

    auto staticSamplers = GetStaticSamplers();

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(
        RootParameters::NumRootParameters,
        rootParameters,
        staticSamplers.size(),
        staticSamplers.data(),
        rootSignatureFlags
    );

    m_RootSignature = m_Device->CreateRootSignature(rootSignatureDescription.Desc_1_1);

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendDesc;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencil;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    	CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSFormat;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.LogicOpEnable = false;          // Disable logic operations (typical for blending)
	rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;     // Source color factor
	rtBlendDesc.DestBlend = D3D12_BLEND_ONE;    // Destination color factor
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;   // Add source and destination for color
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;     // Source alpha factor
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;    // Destination alpha factor
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;   // Add source and destination for alpha
	rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;       // No-op since LogicOpEnable is false
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // Write to all channels (RGBA)
	
	CD3DX12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.RenderTarget[0] = rtBlendDesc;

    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = backBufferFormat;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);
	rasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	rasterizerState.FrontCounterClockwise = TRUE;

	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	pipelineStateStream.BlendDesc = blendDesc;
    pipelineStateStream.RasterizerState = rasterizerState;
	pipelineStateStream.DepthStencil = depthStencilDesc;
    pipelineStateStream.InputLayout = dx12lib::VertexPosition::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.DSFormat = depthBufferFormat;
    pipelineStateStream.SampleDesc = sampleDesc;

    m_PipelineStateObject = m_Device->CreatePipelineStateObject(pipelineStateStream);

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

void PointLightsPSO::Apply(dx12lib::CommandList& commandList)
{
    commandList.SetPipelineState(m_PipelineStateObject);
    commandList.SetGraphicsRootSignature(m_RootSignature);
    commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (m_DirtyFlags & DF_PassData)
    {
        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerPassDataCB, m_PassData);
    }
	
	if (m_DirtyFlags & DF_LightData)
	{
		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PointLightCB, m_LightData);
	}
	
	if (m_DirtyFlags & DF_ObjectData)
	{
		commandList.SetGraphicsDynamicConstantBuffer(RootParameters::LightVolumeCB, m_ObjectData);
	}

    if (m_GBuffer)
    {
        using TextureType = GBuffer::TextureType;
        BindTexture(commandList, RootParameters::GBufferTextures, 0, m_GBuffer->GetTexture(TextureType::AlbedoOpacity), m_DefaultSRV);
        BindTexture(commandList, RootParameters::GBufferTextures, 1, m_GBuffer->GetTexture(TextureType::NormalSpec), m_DefaultSRV);
        BindTexture(commandList, RootParameters::GBufferTextures, 2, m_GBuffer->GetTexture(TextureType::Reflectance), m_DefaultSRV);
        BindTexture(commandList, RootParameters::GBufferTextures, 3, m_GBuffer->GetTexture(TextureType::EmissiveAmbient), m_DefaultSRV);
        BindTexture(commandList, RootParameters::GBufferTextures, 4, m_GBuffer->GetTexture(TextureType::Depth), m_DefaultSRV);
    }
    else
    {
        printf("[PointLightsPSO::Apply] Error! No GBuffer set!!!\n");
    }

    m_DirtyFlags = DF_None;
}