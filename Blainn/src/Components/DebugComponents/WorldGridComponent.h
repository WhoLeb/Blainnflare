#pragma once

#include "Core/Application.h"
#include "Components/Component.h"
#include "Components/ComponentManager.h"
#include "DX12/DXStaticMesh.h"

namespace Blainn
{
	
	class WorldGridComponent : public Component
	{
	public:
		WorldGridComponent()
		{
			ComponentManager::Get().RegisterComponent(this);

			m_PointList = CreateLineList();
		}

		void Render()
		{
			auto cmdList = Application::Get().GetRenderingContext()->GetCommandList();
			m_PointList->Bind();

			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

			m_PointList->Draw();
		}

	private:
		std::shared_ptr<DXStaticMesh> CreateLineList();
	private:
		std::shared_ptr<DXStaticMesh> m_PointList;
	};
}
