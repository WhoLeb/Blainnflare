#pragma once

#include "Components/Component.h"

namespace Blainn
{
	class DXStaticMesh;
	
	class WorldGridComponent : public Component<WorldGridComponent>
	{
	public:
		WorldGridComponent(std::shared_ptr<GameObject> owner);

		~WorldGridComponent();

		void OnAttach() override;

		void Render();

	private:
		std::shared_ptr<DXStaticMesh> CreateLineList();
	private:
		std::shared_ptr<DXStaticMesh> m_PointList;
	};
}
