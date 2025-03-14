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

	public:
		static std::shared_ptr<DXModel> ColoredCube(float side = 1.f, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f})
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateCube(side, color));
		}

		static std::shared_ptr<DXModel> ColoredSphere(float radius = 1.f, UINT sliceCount = 10, UINT stackCount = 10, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f})
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateSphere(radius, sliceCount, stackCount, color));
		}
		static std::shared_ptr<DXModel> ColoredCapsule(float radius = .5f, float height = 2.f, UINT sliceCount = 10, UINT sphereStackCount = 10, UINT cylinderStackCount = 2, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f})
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateCapsule(radius, height, sliceCount, sphereStackCount, cylinderStackCount, color));
		}

		static std::shared_ptr<DXModel> ColoredTorus(float majorRadius = 2.f, float minorRadius = 1.f, UINT majorSegments = 20, UINT minorSegments = 10, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f})
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateTorus(majorRadius, minorRadius, majorSegments, minorSegments, color));
		}
		static std::shared_ptr<DXModel> ColoredTorusKnot(int p = 1, int q = 0, float radius = 2.f, float tubeRadius = .3f, UINT curveSegments = 20, UINT tubeSegments = 10, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f})
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateTorusKnot(p, q, radius, tubeRadius, curveSegments, tubeSegments, color));
		}

		static std::shared_ptr<DXModel> ColoredPyramid(float width = 1.f, float height = 1.f, const DirectX::SimpleMath::Color& color = { 1.f, 0.f, 1.f, 1.f })
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreatePyramid(width, height, color));
		}
		static std::shared_ptr<DXModel> ColoredCone(float bottomRadius = .5f, float height = 1.f, UINT sliceCount = 5, const DirectX::SimpleMath::Color& color = { 1.f, 0.f, 1.f, 1.f })
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateCone(bottomRadius, height, sliceCount, color));
		}
		static std::shared_ptr<DXModel> ColoredCylinder(float bottomRadius = .5f, float topRadius = .5f, float height = 1.f, UINT sliceCount = 10, UINT stackCount = 1, const DirectX::SimpleMath::Color& color = { 1.f, 0.f, 1.f, 1.f })
		{
			return std::make_shared<DXModel>(DXStaticMesh::CreateCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, color));
		}
	};
}
