#include "pch.h"
#include "ShadowMap.h"

#include "dx12lib/Device.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"

using namespace Blainn;
using namespace dx12lib;

ShadowMap::ShadowMap(
	std::shared_ptr<dx12lib::Device> Device,
	uint32_t width,
	uint32_t height,
	DXGI_FORMAT format,      // DXGI_FORMAT_R32_TYPELESS
	DXGI_FORMAT dsvFormat,   // DXGI_FORMAT_D32_FLOAT
	DXGI_FORMAT rtvFormat)   // DXGI_FORMAT_R32_FLOAT
{
	m_Device = Device;
	m_Width = width;
	m_Height = height;
	m_Format = format;
	m_dsvFormat = dsvFormat;
	m_rtvFormat = rtvFormat;

	m_Viewport = { 0.f, 0.f, (float)width, (float)width, 0.f, 1.f };

	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		m_Format,
		width,
		height,
		1, // array size
		1, // mip levels
		1, // sample count
		0, // quality
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_dsvFormat;
	optClear.DepthStencil.Depth = 1.f;
	optClear.DepthStencil.Stencil = 0;

	m_ShadowMapTex = Device->CreateTexture(texDesc, &optClear);

	auto dsvDesc = std::make_shared<D3D12_DEPTH_STENCIL_VIEW_DESC>();
	dsvDesc->Format = m_dsvFormat;
	dsvDesc->ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc->Flags = D3D12_DSV_FLAG_NONE;
	m_ShadowMapTex->CreateDepthStencilView(dsvDesc);

	auto srvDesc = std::make_shared<D3D12_SHADER_RESOURCE_VIEW_DESC>();
	srvDesc->Format = m_rtvFormat;
	srvDesc->ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc->Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc->Texture2D.MipLevels = 1;

	m_ShadowMapTex->CreateShaderResourceView(srvDesc);

	m_ShadowMapRT.AttachTexture(dx12lib::AttachmentPoint::DepthStencil, m_ShadowMapTex);
}
