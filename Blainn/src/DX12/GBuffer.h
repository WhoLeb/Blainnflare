#pragma once

#include <dx12lib/RenderTarget.h>


namespace Blainn
{
	class GPassPSO;
}

namespace dx12lib
{
	class CommandList;
	class Device;
	class Texture;
}

namespace Blainn
{
	class GBuffer
	{
	public:
		enum TextureType
		{
			AlbedoOpacity,
			NormalSpec,
			Reflectance,
			EmissiveAmbient,
			Depth
		};
		
		GBuffer(std::shared_ptr<dx12lib::Device> device, UINT width, UINT height);

		void ClearRenderTarget(const std::shared_ptr<dx12lib::CommandList>& commandList);

		const dx12lib::RenderTarget& GetRenderTarget() const { return m_RenderTarget; } 
		dx12lib::RenderTarget& GetRenderTarget() { return m_RenderTarget; }

		std::shared_ptr<GPassPSO> GetGPassPSO() { return m_GPassPSO; }

		void SetTexture(TextureType type, const std::shared_ptr<dx12lib::Texture>& texture);
		std::shared_ptr<dx12lib::Texture> GetTexture(TextureType type) const;

	protected:
		dx12lib::RenderTarget m_RenderTarget;

		FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		std::shared_ptr<GPassPSO> m_GPassPSO;

		using TextureMap = std::map<TextureType, std::shared_ptr<dx12lib::Texture>>;
		TextureMap m_Textures;
	};
}
