#include "pch.h"
#include "CascadeShadowMaps.h"

#include <dx12lib/CommandList.h>
#include <dx12lib/Device.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Texture.h>

using namespace dx12lib;
using namespace Blainn;

CascadeShadowMaps::CascadeShadowMaps()
	: m_Slices(CascadeSlice::NumSlices)
	, m_RenderTargets(CascadeSlice::NumSlices)
	, m_Sizes(CascadeSlice::NumSlices, {0, 0})
{}

Blainn::CascadeShadowMaps::CascadeShadowMaps(CascadeShadowMaps& copy)
	: m_Slices(copy.m_Slices)
{}

void CascadeShadowMaps::AttachShadowMap(CascadeSlice slice, std::shared_ptr<dx12lib::Texture> texture)
{
	m_Slices[slice] = texture;

	if (texture && texture->GetD3D12Resource())
	{
		auto desc = texture->GetD3D12ResourceDesc();

		m_Sizes[slice].x = static_cast<uint32_t>(desc.Width);
		m_Sizes[slice].y = static_cast<uint32_t>(desc.Height);

		m_RenderTargets[slice].AttachTexture(dx12lib::AttachmentPoint::DepthStencil, texture);
	}
}

std::shared_ptr<dx12lib::Texture> CascadeShadowMaps::GetSlice(CascadeSlice slice) const
{
	return m_Slices[slice];
}

const std::vector<std::shared_ptr<dx12lib::Texture>>& CascadeShadowMaps::GetSlices() const
{
	return m_Slices;
}

const dx12lib::RenderTarget& Blainn::CascadeShadowMaps::GetRenderTarget(CascadeSlice slice) const
{
	return m_RenderTargets[slice];
}

dx12lib::RenderTarget& Blainn::CascadeShadowMaps::GetRenderTarget(CascadeSlice slice)
{
	return m_RenderTargets[slice];
}

const std::vector<dx12lib::RenderTarget>& Blainn::CascadeShadowMaps::GetRenderTargets() const
{
	return m_RenderTargets;
}

void Blainn::CascadeShadowMaps::TransitionTo(std::shared_ptr<dx12lib::CommandList> commandList, D3D12_RESOURCE_STATES state)
{
	for (int i = CascadeSlice::Slice0; i < CascadeSlice::NumSlices; ++i)
	{
		commandList->TransitionBarrier(m_Slices[i], state);
	}
}

DirectX::XMUINT2 CascadeShadowMaps::GetSize(CascadeSlice slice)
{
	if (slice < CascadeSlice::NumSlices)
		return m_Sizes[slice];
	else
		return { 0, 0 };
}

std::vector<DXGI_FORMAT> CascadeShadowMaps::GetShadowMapFormats() const
{
	std::vector<DXGI_FORMAT> smFormats(CascadeSlice::NumSlices);
	for (int i = CascadeSlice::Slice0; i < CascadeSlice::NumSlices; ++i)
	{
		auto texture = m_Slices[i];
		if (texture)
			smFormats[i] = texture->GetD3D12ResourceDesc().Format;
	}

	return smFormats;
}

void CascadeShadowMaps::Reset()
{
	m_Slices = ShadowMapList(CascadeSlice::NumSlices);
}

ShadowMapPSO::ShadowMapPSO(
	std::shared_ptr<dx12lib::Device> device,
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob,
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType
)
	: m_Device(device)
	, m_DirtyFlags(DF_All)
	, m_PrimitiveTopologyType(primitiveTopologyType)
{
	CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
	rootParameters[RootParameters::PerObjectDataCB	].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootParameters::PerPassDataCB	].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(
        RootParameters::NumRootParameters,
        rootParameters,
        0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

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

    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 0;

    CD3DX12_RASTERIZER_DESC rasterizerState(D3D12_DEFAULT);

    pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.RasterizerState = rasterizerState;
    pipelineStateStream.InputLayout = VertexPositionNormalTangentBitangentTexture::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = primitiveTopologyType;
    pipelineStateStream.DSVFormat = depthBufferFormat;
    pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.SampleDesc = { 1, 0 };

    m_PipelineStateObject = m_Device->CreatePipelineStateObject(pipelineStateStream);
}

void ShadowMapPSO::Apply(dx12lib::CommandList& commandList)
{
	commandList.SetPipelineState(m_PipelineStateObject);
	commandList.SetGraphicsRootSignature(m_RootSignature);

    if (m_DirtyFlags & DF_PerObjectData)
    {
        PerObjectData m;
        m.WorldMatrix = m_ObjectData.WorldMatrix;

        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerObjectDataCB, m);
    }

    if (m_DirtyFlags & DF_PerPassData)
    {
        commandList.SetGraphicsDynamicConstantBuffer(RootParameters::PerPassDataCB, m_PassData);
    }

	m_DirtyFlags = DF_None;
}
