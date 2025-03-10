#include "Planet.h"

#include "windows.h"
#include <unordered_map>

#include "Components/ActorComponents/TransformComponent.h"

namespace solar
{
	void Planet::OnUpdate(const Blainn::GameTimer& gt)
	{
		auto transform = GetComponent<Blainn::TransformComponent>();
		auto prevRot = transform->GetLocalQuat();
		DirectX::SimpleMath::Quaternion deltaQuat =
			DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(.5f * gt.DeltaTime(), 0.f, 0.f);
		transform->SetLocalQuat(prevRot * deltaQuat);
	}
}
