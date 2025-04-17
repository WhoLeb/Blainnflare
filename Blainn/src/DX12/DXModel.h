#pragma once

#include <cassert>
#include <d3d12.h>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <wrl.h>

#include "SimpleMath.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

namespace dx12lib
{
	class CommandList;
	class Scene;
	class Texture;
	class Visitor;
}

namespace Blainn
{
	class DXMaterial;
	class DXStaticMesh;
	class DXTexture;
	class SceneVisitor;

	class DXModel
	{
	public:
		DXModel(const std::filesystem::path& modelFilePath);
		//DXModel(std::shared_ptr<DXStaticMesh> staticMesh, std::shared_ptr<DXMaterial> materaial = nullptr);

		void Render(dx12lib::Visitor& sceneVisitor);

		auto GetScene() { return m_Scene; }
		auto GetScene() const { return m_Scene; }

	private:
		std::filesystem::path m_ModelFilepath;

		std::shared_ptr<dx12lib::Scene> m_Scene;

	//public:
	//	static std::shared_ptr<DXModel> ColoredCube(float side = 1.f, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f}, std::shared_ptr<DXMaterial> material = nullptr);

	//	static std::shared_ptr<DXModel> ColoredSphere(float radius = 1.f, UINT sliceCount = 10, UINT stackCount = 10, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f}, std::shared_ptr<DXMaterial> material = nullptr);
	//	static std::shared_ptr<DXModel> ColoredCapsule(float radius = .5f, float height = 2.f, UINT sliceCount = 10, UINT sphereStackCount = 10, UINT cylinderStackCount = 2, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f}, std::shared_ptr<DXMaterial> material = nullptr);

	//	static std::shared_ptr<DXModel> ColoredTorus(float majorRadius = 2.f, float minorRadius = 1.f, UINT majorSegments = 20, UINT minorSegments = 10, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f}, std::shared_ptr<DXMaterial> material = nullptr);
	//	static std::shared_ptr<DXModel> ColoredTorusKnot(int p = 1, int q = 0, float radius = 2.f, float tubeRadius = .3f, UINT curveSegments = 20, UINT tubeSegments = 10, const DirectX::SimpleMath::Color& color = {1.f, 0.f, 1.f, 1.f}, std::shared_ptr<DXMaterial> material = nullptr);

	//	static std::shared_ptr<DXModel> ColoredPyramid(float width = 1.f, float height = 1.f, const DirectX::SimpleMath::Color& color = { 1.f, 0.f, 1.f, 1.f }, std::shared_ptr<DXMaterial> material = nullptr);
	//	static std::shared_ptr<DXModel> ColoredCone(float bottomRadius = .5f, float height = 1.f, UINT sliceCount = 5, const DirectX::SimpleMath::Color& color = { 1.f, 0.f, 1.f, 1.f }, std::shared_ptr<DXMaterial> material = nullptr);
	//	static std::shared_ptr<DXModel> ColoredCylinder(float bottomRadius = .5f, float topRadius = .5f, float height = 1.f, UINT sliceCount = 10, UINT stackCount = 1, const DirectX::SimpleMath::Color& color = { 1.f, 0.f, 1.f, 1.f }, std::shared_ptr<DXMaterial> material = nullptr);
	};
}
