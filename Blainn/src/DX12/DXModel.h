#pragma once

#include "DXStaticMesh.h"
#include "DXTexture.h"

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Blainn
{
	class DXModel
	{
	public:
		DXModel(std::filesystem::path modelFilePath);
		DXModel(std::filesystem::path modelFilePath, const std::vector<std::filesystem::path>& texturePaths);
		DXModel(std::shared_ptr<DXStaticMesh> staticMesh);
		DXModel(std::shared_ptr<DXStaticMesh> staticMesh, const std::vector<std::shared_ptr<DXTexture>>& textures);

		void Render();
	private:
		std::shared_ptr<DXStaticMesh> m_Mesh;
		std::unordered_map<std::string, std::shared_ptr<DXTexture>> m_Textures;
	};
}
