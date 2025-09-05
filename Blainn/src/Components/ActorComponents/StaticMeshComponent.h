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
		static std::shared_ptr<StaticMeshComponent> Create(
			std::shared_ptr<GameObject> owner,
			const std::filesystem::path& filepath);

		~StaticMeshComponent();

		void OnAttach() override;

		void OnRender(dx12lib::Visitor& frameInfo);

		std::shared_ptr<DXModel> GetModel() const;

		const std::vector<std::weak_ptr<GameObject>>& GetOwners() const { return m_Owners; }
		
	private:
		StaticMeshComponent(std::shared_ptr<GameObject> owner, const std::filesystem::path& filepath);

	private:
		std::shared_ptr<DXModel> m_Model;
		std::vector<std::weak_ptr<GameObject>> m_Owners;
	};
}
