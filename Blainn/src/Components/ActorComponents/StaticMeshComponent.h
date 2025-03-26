#pragma once

#include "Components/ComponentManager.h"
#include "Components/Component.h"
#include "Core/CBIndexManager.h"
#include "Core/GameObject.h"

#include "DX12/DXModel.h"

#include <filesystem>

namespace Blainn
{

	class StaticMeshComponent : public Blainn::Component<StaticMeshComponent>
	{
		friend class Scene;
		using Super = Blainn::Component<StaticMeshComponent>;

	public:
		StaticMeshComponent(std::filesystem::path filepath)
		 : Blainn::Component<StaticMeshComponent>()
		{
			m_Model = (std::make_shared<DXModel>(filepath));
		}
		StaticMeshComponent(std::shared_ptr<DXModel> model)
			: m_Model(model)
		{}

		~StaticMeshComponent()
		{
			m_Model = nullptr;
		}

		void OnAttach() override
		{
			Super::OnAttach();
			Blainn::CBIndexManager::Get().AssignCBIdx(GetOwner()->GetUUID());
		}

		void OnRender(DXFrameInfo& frameInfo) { m_Model->Render(frameInfo); };

		std::shared_ptr<DXModel> GetModel() const { return m_Model; }

	private:
		std::shared_ptr<DXModel> m_Model;
	};
}
