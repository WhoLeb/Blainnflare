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

Blainn::StaticMeshComponent::StaticMeshComponent(std::filesystem::path filepath)
	: Blainn::Component<StaticMeshComponent>()
{
	m_Model = (std::make_shared<DXModel>(filepath));
}

//Blainn::StaticMeshComponent::StaticMeshComponent(std::shared_ptr<DXModel> model)
//	: m_Model(model)
//{
//}

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
