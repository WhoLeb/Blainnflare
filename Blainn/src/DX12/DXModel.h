#pragma once

#include "DXStaticMesh.h"
#include "DXTexture.h"

#include <filesystem>
#include <string>
#include <unordered_map>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Blainn
{
	class DXModel
	{
	public:
		DXModel(const std::filesystem::path& modelFilePath);
		DXModel(std::shared_ptr<DXStaticMesh> staticMesh);

		void Render();
	private:
		void LoadFromFile(const std::filesystem::path& modelFilePath);

		void ProcessNode(aiNode* node, const aiScene* scene);
		std::shared_ptr<DXStaticMesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);

	private:

		struct SubMeshData
		{
			std::shared_ptr<DXStaticMesh> Mesh;
			// probably replace with material
			std::vector<std::shared_ptr<DXTexture>> Textures;
		};

		std::vector<SubMeshData> m_SubMeshes;

	};
}
