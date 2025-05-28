#include "pch.h"
#include "GBuffer.h"

#include "DXShader.h"
#include "GPassPSO.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/Device.h"
#include "dx12lib/Texture.h"

using namespace dx12lib;
using namespace Blainn;

Blainn::GBuffer::GBuffer(std::shared_ptr<dx12lib::Device> device, UINT width, UINT height)
{
	auto vertexShader = DXShader(L"src\\Shaders\\BasicVS.hlsl", true, nullptr, "main", "vs_5_1");
	auto pixelShader = DXShader(L"src\\Shaders\\DeferredShading\\PS_GBuffer.hlsl", true, nullptr, "PS_GBuffer", "ps_5_1");

	m_GPassPSO = std::make_shared<GPassPSO>(device, vertexShader.GetByteCode(), pixelShader.GetByteCode());

	//DXGI_SAMPLE_DESC sampleDesc = device->GetMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM);
	DXGI_SAMPLE_DESC sampleDesc = {1, 0};

	auto AlbedoOpacityDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1,
		sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto NormalSpecDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R10G10B10A2_UNORM, width, height, 1, 1,
		sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto ReflectanceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1,
		sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	auto EmissiveAmbientDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R10G10B10A2_UNORM, width, height, 1, 1,
		sampleDesc.Count, sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	D3D12_CLEAR_VALUE colorClearValue;
	colorClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorClearValue.Color[0] = 0.0f;
	colorClearValue.Color[1] = 0.0f;
	colorClearValue.Color[2] = 0.0f;
	colorClearValue.Color[3] = 1.0f;
	
	D3D12_CLEAR_VALUE otherClearValue;
	otherClearValue.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	otherClearValue.Color[0] = 0.0f;
	otherClearValue.Color[1] = 0.0f;
	otherClearValue.Color[2] = 0.0f;
	otherClearValue.Color[3] = 1.0f;

	auto AlbedoOpacityTexture = device->CreateTexture(AlbedoOpacityDesc, &colorClearValue);
	AlbedoOpacityTexture->SetName(L"AlbedoOpacityTexture");
	
	auto NormalSpecTexture = device->CreateTexture(NormalSpecDesc, &otherClearValue);
	NormalSpecTexture->SetName(L"NormalSpecTexture");
	
	auto ReflectanceTexture = device->CreateTexture(ReflectanceDesc, &colorClearValue);
	ReflectanceTexture->SetName(L"ReflectanceTexture");
	
	auto EmissiveAmbientTexture = device->CreateTexture(EmissiveAmbientDesc, &otherClearValue);
	EmissiveAmbientTexture->SetName(L"EmissiveAmbientTexture");

	auto DepthDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, width, height, 1, 1,
		1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE depthClearValue;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil = {1.0f, 0};

	auto DepthTexture = device->CreateTexture(DepthDesc, &depthClearValue);
	DepthTexture->SetName(L"Depth RT");

	auto dsvDesc = std::make_shared<D3D12_DEPTH_STENCIL_VIEW_DESC>();
	dsvDesc->Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc->ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc->Flags = D3D12_DSV_FLAG_NONE;
	DepthTexture->CreateDepthStencilView(dsvDesc);
	
	auto srvDesc = std::make_shared<D3D12_SHADER_RESOURCE_VIEW_DESC>();
	srvDesc->Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc->ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc->Texture2D.MipLevels = 1;
	srvDesc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	DepthTexture->CreateShaderResourceView(srvDesc);

	m_RenderTarget.AttachTexture(AttachmentPoint::Color0, AlbedoOpacityTexture);
	m_RenderTarget.AttachTexture(AttachmentPoint::Color1, NormalSpecTexture);
	m_RenderTarget.AttachTexture(AttachmentPoint::Color2, ReflectanceTexture);
	m_RenderTarget.AttachTexture(AttachmentPoint::Color3, EmissiveAmbientTexture);
	m_RenderTarget.AttachTexture(AttachmentPoint::DepthStencil, DepthTexture);

	SetTexture(TextureType::AlbedoOpacity, AlbedoOpacityTexture);
	SetTexture(TextureType::NormalSpec, NormalSpecTexture);
	SetTexture(TextureType::Reflectance, ReflectanceTexture);
	SetTexture(TextureType::EmissiveAmbient, EmissiveAmbientTexture);
	SetTexture(TextureType::Depth, DepthTexture);
}

void GBuffer::ClearRenderTarget(const std::shared_ptr<CommandList>& commandList)
{
	for (int attPoint = AttachmentPoint::Color0; attPoint < AttachmentPoint::DepthStencil; attPoint++)
	{
		if (auto tex = m_RenderTarget.GetTexture(AttachmentPoint(attPoint)))
			commandList->ClearTexture(tex, clearColor);
	}
	commandList->ClearDepthStencilTexture(m_RenderTarget.GetTexture(AttachmentPoint::DepthStencil),
											D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL);
}

void GBuffer::SetTexture(TextureType type, const std::shared_ptr<dx12lib::Texture>& texture)
{
	m_Textures[type] = texture;
}

std::shared_ptr<dx12lib::Texture> GBuffer::GetTexture(TextureType type) const
{
	TextureMap::const_iterator it = m_Textures.find(type);
	if (it != m_Textures.end())
		return it->second;
	
	return nullptr;
}
