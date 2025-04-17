#pragma once

#include "Components/Component.h"

#include <filesystem>

namespace dx12lib
{
	class Scene;
	class Visitor;
}

namespace Blainn
{
	class DXModel;
	class SceneVisitor;

	class StaticMeshComponent : public Blainn::Component<StaticMeshComponent>
	{
		friend class Scene;
		using Super = Blainn::Component<StaticMeshComponent>;

	public:
		StaticMeshComponent(std::filesystem::path filepath);
		//StaticMeshComponent(std::shared_ptr<DXModel> model);

		~StaticMeshComponent();

		void OnAttach() override;

		void OnRender(dx12lib::Visitor& frameInfo);;

		std::shared_ptr<DXModel> GetModel() const;
	private:
		std::shared_ptr<DXModel> m_Model;
	};
}
