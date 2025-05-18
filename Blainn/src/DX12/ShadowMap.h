#pragma once

#include "dx12lib/RenderTarget.h"

namespace dx12lib
{
	class Device;
	class Texture;
	class RenderTarget;
}

namespace Blainn
{
	class ShadowMap
	{
	public:
		ShadowMap(std::shared_ptr<dx12lib::Device> Device,
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format = DXGI_FORMAT_R32_TYPELESS,
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT,
			DXGI_FORMAT rtvFormat = DXGI_FORMAT_R32_FLOAT);

		ShadowMap(const ShadowMap& other) = delete;
		ShadowMap(const ShadowMap&& other) = delete;

		ShadowMap& operator=(const ShadowMap& other) = delete;
		ShadowMap& operator=(const ShadowMap&& other) = delete;

		~ShadowMap() = default;

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		D3D12_VIEWPORT GetViewport() const { return m_Viewport; }

		const dx12lib::RenderTarget& GetRenderTarget() const { return m_ShadowMapRT; }
		dx12lib::RenderTarget& GetRenderTarget() { return m_ShadowMapRT; }
		std::shared_ptr<dx12lib::Texture> GetTexture() const { return m_ShadowMapTex; }

	private:
		std::shared_ptr<dx12lib::Device> m_Device;

		D3D12_VIEWPORT m_Viewport;
		uint32_t m_Width;
		uint32_t m_Height;

		DXGI_FORMAT m_Format = DXGI_FORMAT_R32_TYPELESS;
		DXGI_FORMAT m_dsvFormat = DXGI_FORMAT_D32_FLOAT;
		DXGI_FORMAT m_rtvFormat = DXGI_FORMAT_R32_FLOAT;

		dx12lib::RenderTarget m_ShadowMapRT;
		std::shared_ptr<dx12lib::Texture> m_ShadowMapTex;
	};
}
