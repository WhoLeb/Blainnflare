#pragma once

#include "Core/UUID.h"

#include <filesystem>

namespace dx12lib
{
	class CommandList;
}

namespace Blainn
{
	class DXTexture
	{
	public:
		DXTexture(const std::filesystem::path& filepath);
		~DXTexture();
		UUID uuid;

		void Bind(std::shared_ptr<dx12lib::CommandList> commandList);
		
		std::filesystem::path FilePath;
	private:
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}
