#include "pch.h"
#include "StaticMeshComponent.h"

#include "Components/ComponentManager.h"
#include "Core/CBIndexManager.h"
#include "Core/GameObject.h"
#include "Core/Application.h"
#include "DX12/DXRenderingContext.h"
#include "DX12/DXModel.h"
#include "DX12/DXSceneVisitor.h"

#include <dx12lib/Device.h>

using namespace Blainn;

Blainn::StaticMeshComponent::StaticMeshComponent(std::shared_ptr<GameObject> owner, const std::filesystem::path& filepath)
	: Super(owner)
{
	m_Model = (std::make_shared<DXModel>(filepath));
	m_Owners.push_back(owner);
}

std::shared_ptr<StaticMeshComponent> Blainn::StaticMeshComponent::Create(std::shared_ptr<GameObject> owner, const std::filesystem::path& filepath)
{
	auto& allSMs = ComponentManager::Get().GetComponents<StaticMeshComponent>();
	auto it = std::find_if(allSMs.begin(), allSMs.end(),
			[&filepath](std::shared_ptr<StaticMeshComponent> comp)->bool
			{
				return comp->GetModel()->GetPath() == filepath;
			});

	if (it != allSMs.end())
	{
		(*it)->m_Owners.push_back(owner);
		return *it;
	}

	struct Enabler : StaticMeshComponent {
		Enabler(std::shared_ptr<GameObject> o, const std::filesystem::path& p)
			: StaticMeshComponent(std::move(o), p) { }
	};

	auto newComp = std::make_shared<Enabler>(owner, filepath);
	return newComp;
}

Blainn::StaticMeshComponent::~StaticMeshComponent()
{
	m_Model = nullptr;
}

void Blainn::StaticMeshComponent::OnAttach()
{
	Super::OnAttach();
	Blainn::CBIndexManager::Get().AssignCBIdx(GetOwner()->GetUUID());
}

void Blainn::StaticMeshComponent::OnRender(dx12lib::Visitor& sceneVisitor)
{
	m_Model->Render(sceneVisitor);
}

std::shared_ptr<DXModel> Blainn::StaticMeshComponent::GetModel() const
{
	return m_Model;
}
