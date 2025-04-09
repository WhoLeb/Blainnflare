#pragma once

#include "Core/UUID.h"

#include <filesystem>

namespace Blainn
{
	class DXTexture
	{
	public:
		DXTexture(const std::filesystem::path& filepath);
		~DXTexture();
		UUID uuid;

		void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList);
		
		std::filesystem::path FilePath;
	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}
