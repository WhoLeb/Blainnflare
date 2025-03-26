#include "pch.h"
#include "DXModel.h"

#include "Core/Application.h"
#include "Core/MaterialIndexManager.h"
#include "DXMaterial.h"
#include "DXStaticMesh.h"
#include "DXTexture.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <DirectXTex.h>

namespace Blainn
{
	Blainn::DXModel::DXModel(const std::filesystem::path& modelFilePath)
		: m_ModelFilepath(modelFilePath)
	{
		LoadFromFile(modelFilePath);
	}

	Blainn::DXModel::DXModel(std::shared_ptr<DXStaticMesh> staticMesh, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> materials;
		if (material)
			materials = material;
		else
			materials = std::make_shared<DXMaterial>();

		m_SubMeshes.push_back({ staticMesh, materials });
	}

	void Blainn::DXModel::Render(DXFrameInfo& frameInfo)
	{
		for (auto& sm : m_SubMeshes)
		{
			sm.Mesh->Bind();
			//bind textures, then draw mesh

			auto objCBAddr = frameInfo.ObjectCBAddress;
			auto matCBAddr = frameInfo.MaterialCBAddress;
			matCBAddr += MaterialIndexManager::Get().GetMatIdx(sm.Material->uuid) * frameInfo.MatCBSize;

			frameInfo.CommandList->SetGraphicsRootConstantBufferView(1, objCBAddr);
			frameInfo.CommandList->SetGraphicsRootConstantBufferView(2, matCBAddr);
			if (sm.DiffuseTexture)
				sm.DiffuseTexture->Bind(frameInfo.CommandList);
			sm.Mesh->Draw();
		}
	}

	std::vector<DXMaterial*> DXModel::GetMaterials()
	{
		std::vector<DXMaterial*> materials;
		materials.reserve(m_SubMeshes.size());

		for (auto& submesh : m_SubMeshes)
			materials.push_back(submesh.Material.get());

		return materials;
	}

	void Blainn::DXModel::LoadFromFile(const std::filesystem::path& modelFilePath)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			modelFilePath.string(),
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals
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

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			subMeshData.Material = std::make_shared<DXMaterial>();
			if (material)
			{
				auto diffuseTextures = LoadMaterialTextures(
					material,
					aiTextureType_DIFFUSE,
					scene
				);
				if(!diffuseTextures.empty())
					subMeshData.DiffuseTexture = diffuseTextures[0];
			}

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

			if (mesh->mColors[0] != NULL
				//&& mesh->mColors[0] != NULL
				)
				v.Color = DirectX::SimpleMath::Vector4(
					mesh->mColors[0][i].r,
					mesh->mColors[0][i].g,
					mesh->mColors[0][i].b,
					mesh->mColors[0][i].a
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

	std::vector<std::shared_ptr<DXTexture>> DXModel::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const aiScene* scene)
	{

		std::vector<std::shared_ptr<DXTexture>> textures;

		UINT textureCount = mat->GetTextureCount(type);
		for (UINT i = 0; i < textureCount; ++i)
		{
			aiString path;
			if (mat->GetTexture(type, i, &path) == AI_FAILURE)
				continue;

			std::string pathstr = path.C_Str();
			std::filesystem::path texPathName = std::filesystem::path(pathstr).filename();
			std::string texDDSName = texPathName.string() + ".dds";
			std::filesystem::path texDDSPath = m_ModelFilepath.parent_path() / texDDSName;

			DirectX::ScratchImage scratch;
			DirectX::TexMetadata metadata;

			if (path.C_Str()[0] == '*')
			{
				pathstr.erase(pathstr.begin());
				int texId = std::stoi(pathstr);

				aiTexture* texture = scene->mTextures[texId];
				aiString& texName = texture->mFilename;

				texPathName = std::filesystem::path(texName.C_Str()).filename();
				texDDSName = texPathName.string() + ".dds";
				texDDSPath = m_ModelFilepath.parent_path() / texDDSName;

				if (std::filesystem::exists(texDDSPath))
				{
					textures.emplace_back(std::make_shared<DXTexture>(texDDSPath));
					continue;
				}

				size_t sizeInBytes = texture->mWidth;
				const UINT8* compressedData = reinterpret_cast<const UINT8*>(texture->pcData);

				ThrowIfFailed(DirectX::LoadFromWICMemory(
					compressedData, sizeInBytes,
					DirectX::WIC_FLAGS_DEFAULT_SRGB,
					&metadata, scratch
				));
			}
			else
			{
				if (std::filesystem::exists(texDDSPath))
				{
					textures.emplace_back(std::make_shared<DXTexture>(texDDSPath));
					continue;
				}

				ThrowIfFailed(DirectX::LoadFromWICFile(
					(m_ModelFilepath.parent_path() / std::filesystem::path(pathstr)).c_str(),
					DirectX::WIC_FLAGS_DEFAULT_SRGB,
					&metadata, scratch
				));
			}


			DirectX::ScratchImage compressed;
			ThrowIfFailed(DirectX::Compress(
				scratch.GetImages(),
				scratch.GetImageCount(),
				scratch.GetMetadata(),
				DXGI_FORMAT_BC7_UNORM_SRGB,
				DirectX::TEX_COMPRESS_PARALLEL,
				0.5f,
				compressed
			));

			ThrowIfFailed(DirectX::SaveToDDSFile(
				compressed.GetImages(),
				compressed.GetImageCount(),
				compressed.GetMetadata(),
				DirectX::DDS_FLAGS_NONE,
				texDDSPath.c_str()
			));

			textures.emplace_back(std::make_shared<DXTexture>(texDDSPath));
		}

		return textures;
	}

	std::shared_ptr<DXModel> DXModel::ColoredCube(float side, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}

		return std::make_shared<DXModel>(DXStaticMesh::CreateCube(side, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredSphere(float radius, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}
		return std::make_shared<DXModel>(DXStaticMesh::CreateSphere(radius, sliceCount, stackCount, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredCapsule(float radius, float height, UINT sliceCount, UINT sphereStackCount, UINT cylinderStackCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}
		return std::make_shared<DXModel>(DXStaticMesh::CreateCapsule(radius, height, sliceCount, sphereStackCount, cylinderStackCount, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredTorus(float majorRadius, float minorRadius, UINT majorSegments, UINT minorSegments, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		 }
		return std::make_shared<DXModel>(DXStaticMesh::CreateTorus(majorRadius, minorRadius, majorSegments, minorSegments, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredTorusKnot(int p, int q, float radius, float tubeRadius, UINT curveSegments, UINT tubeSegments, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}
		return std::make_shared<DXModel>(DXStaticMesh::CreateTorusKnot(p, q, radius, tubeRadius, curveSegments, tubeSegments, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredPyramid(float width, float height, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}
		return std::make_shared<DXModel>(DXStaticMesh::CreatePyramid(width, height, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredCone(float bottomRadius, float height, UINT sliceCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}
		return std::make_shared<DXModel>(DXStaticMesh::CreateCone(bottomRadius, height, sliceCount, color), mat);
	}

	std::shared_ptr<DXModel> DXModel::ColoredCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	{
		std::shared_ptr<DXMaterial> mat = material;
		if (!material)
		{
			mat = std::make_shared<DXMaterial>();
			mat->DiffuseAlbedo = color;
		}
		return std::make_shared<DXModel>(DXStaticMesh::CreateCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, color), mat);
	}

}
