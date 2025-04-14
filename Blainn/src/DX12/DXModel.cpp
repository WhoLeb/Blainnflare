#include "pch.h"
#include "DXModel.h"

#include "Core/Application.h"
#include "Core/MaterialIndexManager.h"
#include "DX12/DXRenderingContext.h"
#include "DXMaterial.h"
#include "DXSceneVisitor.h"
#include "DXStaticMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <dx12lib/Device.h>
#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Scene.h>
#include <dx12lib/Texture.h>

namespace Blainn
{
	Blainn::DXModel::DXModel(const std::filesystem::path& modelFilePath)
		: m_ModelFilepath(modelFilePath)
	{
		//LoadFromFile(modelFilePath);
		auto& queue = Application::Get().GetRenderingContext()->GetDevice()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
		auto commandList = queue.GetCommandList();
		m_Scene = commandList->LoadSceneFromFile(modelFilePath);
		queue.ExecuteCommandList(commandList);
	}

	//Blainn::DXModel::DXModel(std::shared_ptr<DXStaticMesh> staticMesh, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> materials;
	//	if (material)
	//		materials = material;
	//	else
	//		materials = std::make_shared<DXMaterial>();

	//	m_SubMeshes.push_back({ staticMesh, materials });
	//}

	void Blainn::DXModel::Render(SceneVisitor& sceneVisitor)
	{
		m_Scene->Accept(sceneVisitor);
	}

	//std::shared_ptr<DXModel> DXModel::ColoredCube(float side, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}

	//	return std::make_shared<DXModel>(DXStaticMesh::CreateCube(side, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredSphere(float radius, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}
	//	return std::make_shared<DXModel>(DXStaticMesh::CreateSphere(radius, sliceCount, stackCount, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredCapsule(float radius, float height, UINT sliceCount, UINT sphereStackCount, UINT cylinderStackCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}
	//	return std::make_shared<DXModel>(DXStaticMesh::CreateCapsule(radius, height, sliceCount, sphereStackCount, cylinderStackCount, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredTorus(float majorRadius, float minorRadius, UINT majorSegments, UINT minorSegments, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	 }
	//	return std::make_shared<DXModel>(DXStaticMesh::CreateTorus(majorRadius, minorRadius, majorSegments, minorSegments, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredTorusKnot(int p, int q, float radius, float tubeRadius, UINT curveSegments, UINT tubeSegments, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}
	//	return std::make_shared<DXModel>(DXStaticMesh::CreateTorusKnot(p, q, radius, tubeRadius, curveSegments, tubeSegments, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredPyramid(float width, float height, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}
	//	return std::make_shared<DXModel>(DXStaticMesh::CreatePyramid(width, height, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredCone(float bottomRadius, float height, UINT sliceCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}
	//	return std::make_shared<DXModel>(DXStaticMesh::CreateCone(bottomRadius, height, sliceCount, color), mat);
	//}

	//std::shared_ptr<DXModel> DXModel::ColoredCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, const DirectX::SimpleMath::Color& color, std::shared_ptr<DXMaterial> material)
	//{
	//	std::shared_ptr<DXMaterial> mat = material;
	//	if (!material)
	//	{
	//		mat = std::make_shared<DXMaterial>();
	//		mat->DiffuseAlbedo = color;
	//	}
	//	return std::make_shared<DXModel>(DXStaticMesh::CreateCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, color), mat);
	//}

}
