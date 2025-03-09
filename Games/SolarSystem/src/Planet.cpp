#include "Planet.h"

#include "windows.h"
#include <unordered_map>

#include "Components/ActorComponents/TransformComponent.h"

namespace solar
{
	void Planet::OnUpdate(const Blainn::GameTimer& gt)
	{
		auto transform = GetComponent<Blainn::TransformComponent>();
		auto prevRot = transform->GetWorldRotation();
		transform->SetRotation(prevRot + DirectX::SimpleMath::Vector3{0.f, 0.f, gt.DeltaTime()});
	}
}
