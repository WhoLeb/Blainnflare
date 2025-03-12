#include "pch.h"
#include "OrbitalCameraController.h"

#include "Components/ActorComponents/TransformComponent.h"
#include "Core/GameObject.h"

#include <algorithm>

#include <iostream>

namespace Blainn
{
	void OrbitalInputComponent::HandleInput(const GameTimer& gt)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		auto* owner = GetOwner();
		if (!owner) return;

		auto* transform = owner->GetComponent<TransformComponent>();
		if (!transform) return;

		Vector3 targetPos = Vector3::Zero;
		Quaternion parentQuat = Quaternion::Identity;
		Vector3 parentUp = Vector3::Up;
		Matrix parentMat = Matrix::Identity;
		auto parent = owner->GetParent();
		if (parent)
		{
			if (auto* parentTransform = parent->GetComponent<TransformComponent>())
			{
				targetPos = parentTransform->GetWorldPosition();
				parentQuat = parentTransform->GetWorldQuat();
				parentUp = parentTransform->GetWorldUpVector();
				parentMat = parentTransform->GetWorldMatrix();
			}
		}

		if (Input::IsKeyHeld(KeyCode::W))
			m_Radius -= m_Speed * gt.DeltaTime();
		if (Input::IsKeyHeld(KeyCode::S))
			m_Radius += m_Speed * gt.DeltaTime();
		m_Radius = std::clamp(m_Radius, 1.0f, 100.0f);

		if (Input::IsMouseButtonHeld(MouseButton::Right))
		{
			Input::SetCursorMode(CursorMode::Locked);

			auto [mouseDX, mouseDY] = Input::GetMouseDelta();
			float thetaDeltaDeg = mouseDX * m_Sensitivity;
			float thetaDeltaRad = XMConvertToRadians(thetaDeltaDeg);
			float phiDeltaDeg = mouseDY * m_Sensitivity;
			float phiDeltaRad = XMConvertToRadians(phiDeltaDeg);

			m_Phi += phiDeltaRad;
			m_Theta += thetaDeltaRad;
			m_Phi = std::clamp(m_Phi, 0.01f, XM_PI - 0.01f);

			if (m_Theta > XM_2PI)
				m_Theta -= XM_2PI;
			else if (m_Theta < 0)
				m_Theta += XM_2PI;
		}
		else
		{
			Input::SetCursorMode(CursorMode::Normal);
		}

		Vector3 localOffset = {
			m_Radius * sinf(m_Phi) * cosf(m_Theta),
			m_Radius * cosf(m_Phi),
			m_Radius * sinf(m_Phi) * sinf(m_Theta)
		};

		Vector3 rotatedOffset = Vector3::Transform(localOffset, parentQuat);

		Vector3 camWorldPos = targetPos + rotatedOffset;

		Vector3 forward = (targetPos - camWorldPos);
		if (forward.LengthSquared() > 0.001f)
			forward.Normalize();
		else
			forward = Vector3::UnitZ;

		Vector3 right = Vector3::Up.Cross(forward);
		if(right.LengthSquared() < 0.001f)
		{
			right = Vector3::Right;
		}
		right.Normalize();

		Vector3 up = forward.Cross(right);
		up.Normalize();

		Matrix rotationMatrix(
			right.x, right.y, right.z, 0,
			up.x, up.y, up.z, 0,
			forward.x, forward.y, forward.z, 0,
			0, 0, 0, 1
		);

		auto newQuat = Quaternion::CreateFromRotationMatrix(rotationMatrix);

		if (parent)
		{
			Matrix invParentMat = parentMat.Invert();

			Vector3 localPos = Vector3::Transform(camWorldPos, invParentMat);
			Matrix localMat = rotationMatrix * invParentMat;
			Quaternion invParentQuat = Quaternion::CreateFromRotationMatrix(invParentMat);
			auto localQuat = newQuat * invParentQuat;

			transform->SetLocalPositionQuat(localPos, localQuat);
		}
		else
		{
			transform->SetWorldPosition(camWorldPos);
			transform->SetWorldQuat(newQuat);
		}
	}

}
