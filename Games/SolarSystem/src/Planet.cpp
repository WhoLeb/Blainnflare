#include "Planet.h"

#include "windows.h"
#include <unordered_map>

#include "Components/ActorComponents/TransformComponent.h"

#include <iostream>

namespace solar
{
	void Planet::OnUpdate(const Blainn::GameTimer& gt)
	{
		//auto prevRot = transform->GetLocalQuat();
		//DirectX::SimpleMath::Quaternion deltaQuat =
		//	DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(.5f * gt.DeltaTime(), 0.f, 0.f);

		auto transform = GetComponent<Blainn::TransformComponent>();
		auto prevRot = transform->GetLocalYawPitchRoll();
		prevRot.x += m_RotationSpeed * gt.DeltaTime();
		transform->SetLocalYawPitchRoll(prevRot);

		float angularVelocity = (DirectX::XM_2PI) / m_OrbitalPeriod;
		m_CurrentAngle += m_Direction * (angularVelocity * gt.DeltaTime());
		if (m_CurrentAngle > DirectX::XM_2PI) m_CurrentAngle -= DirectX::XM_2PI;

		float x = m_Radius * cosf(m_CurrentAngle);
		float y = m_Radius * sinf(m_CurrentAngle);

		transform->SetLocalPosition({ x, 0.f, y });
	}
}
