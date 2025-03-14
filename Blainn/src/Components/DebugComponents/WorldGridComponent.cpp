#include "pch.h"
#include "WorldGridComponent.h"

namespace Blainn
{
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

		Application::Get().GetResourceManager()->StartUploadCommands();
		auto mesh = std::make_shared<DXStaticMesh>(verticies);
		Application::Get().GetResourceManager()->EndUploadCommands();
		return mesh;
	}
}
