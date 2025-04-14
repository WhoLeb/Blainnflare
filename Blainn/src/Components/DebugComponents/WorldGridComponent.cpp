#include "pch.h"
#include "WorldGridComponent.h"

#include "Components/ComponentManager.h"
#include "Core/Application.h"
#include "Core/CBIndexManager.h"
#include "Core/GameObject.h"
#include "DX12/DXStaticMesh.h"
#include "DX12/DXResourceManager.h"

namespace Blainn
{
	WorldGridComponent::WorldGridComponent()
	{
		m_PointList = CreateLineList();
	}

	WorldGridComponent::~WorldGridComponent()
	{
	}

	void WorldGridComponent::OnAttach()
	{
		auto owner = GetOwner();
		if (!owner)
		{
			OutputDebugStringW(L"The component didn't have an owning object");
			return;
		}
		CBIndexManager::Get().AssignCBIdx(owner->GetUUID());
	}

	void WorldGridComponent::Render()
	{
		//auto cmdList = Application::Get().GetRenderingContext()->GetCommandList();

		//m_PointList->Bind();

		//m_PointList->Draw();
	}

	std::shared_ptr<DXStaticMesh> Blainn::WorldGridComponent::CreateLineList()
	{
		std::vector<DXStaticMesh::Vertex> verticies;
		for (float i = -50.f; i < 50.f; i+= 0.5f)
		{
			verticies.push_back({
				.Position = {i, 0.f, -50.f},
				.Normal = {0.f, 0.f, 0.f},
				.Color = {0.4f, 0.4f, 0.4f, 1.f},
				.UV = {0.f, 0.f}
			});
			verticies.push_back({
				.Position = {i, 0.f, 50.f},
				.Normal = {0.f, 0.f, 0.f},
				.Color = {0.4f, 0.4f, 0.4f, 1.f},
				.UV = {0.f, 0.f}
			});
			verticies.push_back({
				.Position = {-50.f, 0.f, i},
				.Normal = {0.f, 0.f, 0.f},
				.Color = {0.4f, 0.4f, 0.4f, 1.f},
				.UV = {0.f, 0.f}
			});
			verticies.push_back({
				.Position = {50.f, 0.f, i},
				.Normal = {0.f, 0.f, 0.f},
				.Color = {0.4f, 0.4f, 0.4f, 1.f},
				.UV = {0.f, 0.f}
			});
		}

		auto mesh = std::make_shared<DXStaticMesh>(verticies);
		return mesh;
	}
}
