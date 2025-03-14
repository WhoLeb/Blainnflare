#include "pch.h"
#include "DXModel.h"

#include "Core/Application.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Blainn
{
	Blainn::DXModel::DXModel(const std::filesystem::path& modelFilePath)
	{
		auto resourceManager = Application::Get().GetResourceManager();
		resourceManager->StartUploadCommands();
		LoadFromFile(modelFilePath);
		resourceManager->EndUploadCommands();
	}

	Blainn::DXModel::DXModel(std::shared_ptr<DXStaticMesh> staticMesh)
	{
		m_SubMeshes.push_back({ staticMesh, {nullptr} });
	}

	void Blainn::DXModel::Render()
	{
		for (auto& sm : m_SubMeshes)
		{
			sm.Mesh->Bind();
			//bind textures, then draw mesh

			sm.Mesh->Draw();
		}
	}

	void Blainn::DXModel::LoadFromFile(const std::filesystem::path& modelFilePath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			modelFilePath.string(),
			0
		);

		if (!scene || !scene->mRootNode)
			throw std::runtime_error(std::string("Failed to load model:") + (importer.GetErrorString()));

		ProcessNode(scene->mRootNode, scene);
	}

	void DXModel::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (UINT i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			SubMeshData subMeshData;
			subMeshData.Mesh = ProcessMesh(mesh, scene);

			m_SubMeshes.push_back(std::move(subMeshData));
		}

		for (UINT i = 0; i < node->mNumChildren; ++i)
			ProcessNode(node->mChildren[i], scene);
	}

	std::shared_ptr<DXStaticMesh> DXModel::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<DXStaticMesh::Vertex> vertices;
		vertices.reserve(mesh->mNumVertices);
		for (UINT i = 0; i < mesh->mNumVertices; ++i)
		{
			DXStaticMesh::Vertex v = {};
			v.Position = {
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z
			};
			v.Normal = mesh->HasNormals() ?
				DirectX::SimpleMath::Vector3(
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
				) : DirectX::SimpleMath::Vector3(0, 0, 0);

			v.UV = mesh->mTextureCoords[0] ?
				DirectX::SimpleMath::Vector2(
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y
				) : DirectX::SimpleMath::Vector2(0, 0);

			if (mesh->mColors != nullptr
				&& mesh->mColors[0] != NULL
				)
				v.Color = DirectX::SimpleMath::Vector4(
					mesh->mColors[0]->r,
					mesh->mColors[0]->g,
					mesh->mColors[0]->b,
					mesh->mColors[0]->a
				);
			else
				v.Color = DirectX::SimpleMath::Vector4(1, 0, 1, 1);
			vertices.push_back(v);
		}

		std::vector<UINT32> indices;
		for (UINT i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			for (UINT j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		auto staticMesh = std::make_shared<DXStaticMesh>(
			vertices, indices.empty() ? nullptr : &indices);

		return staticMesh;
	}

}
