#pragma once

#include "Components/Component.h"

#include "Core/GameObject.h"
#include "Components/ActorComponents/CharacterComponents/InputComponent.h"
#include "Components/ActorComponents/TransformComponent.h"

#include "SimpleMath.h"

#include <algorithm>

typedef long LONG;

class KatamariCameraInput : public Blainn::InputComponent
{
public:
	KatamariCameraInput()
	{
		m_MouseMovedHandle = Blainn::Input::OnMouseMovedDelegate.AddRaw(this, &KatamariCameraInput::OnMouseMoved);
		m_MouseButtonPressedHandle = Blainn::Input::OnMouseButtonPressedDelegate.AddRaw(this, &KatamariCameraInput::OnMousePressed);
	}

	~KatamariCameraInput()
	{
		if(m_MouseMovedHandle)
			Blainn::Input::OnMouseMovedDelegate.Remove(m_MouseMovedHandle);
		if(m_KeyPressedHandle)
			Blainn::Input::OnKeyPressedDelegate.Remove(m_KeyPressedHandle);
	}

	void HandleInput(const Blainn::GameTimer& gt)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		using namespace Blainn;

		if ((Input::IsKeyPressed(KeyCode::Escape)
			|| Input::IsKeyHeld(KeyCode::Escape)) && m_bCursorLocked)
		{
			Input::SetCursorMode(CursorMode::Normal);
			m_bCursorLocked = false;
		}

		auto owner = GetOwner();
		if (!owner) return;

		auto transform = owner->GetComponent<Blainn::TransformComponent>();
		if (!transform) return;

		auto parent = owner->GetParent();
		if (!parent) return;

		auto parentTransform = parent->GetComponent<Blainn::TransformComponent>();
		if (!parentTransform) return;

		if (!m_bCursorLocked) return;

		if (Input::IsKeyHeld(KeyCode::W))
		{
			auto forward = transform->GetWorldUpVector();
			forward.y = 0.f;
			forward.Normalize();

			auto pos = parentTransform->GetWorldPosition();
			pos += forward * m_Speed * gt.DeltaTime();

			parentTransform->SetWorldPosition(pos);
		}
		if (Input::IsKeyHeld(KeyCode::S))
		{
			auto forward = transform->GetWorldForwardVector();
			forward.y = 0.f;
			forward.Normalize();

			auto pos = parentTransform->GetWorldPosition();
			pos -= forward * m_Speed * gt.DeltaTime();
			parentTransform->SetWorldPosition(pos);
		}
	}

	void OnMouseMoved(LONG xDelta, LONG yDelta)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		auto owner = GetOwner();
		if (!owner) return;

		auto transform = owner->GetComponent<Blainn::TransformComponent>();
		if (!transform) return;

		Vector3 targetPos = Vector3::Zero;
		if (auto parent = owner->GetParent())
		{
			if (auto parentTransform = parent->GetComponent<Blainn::TransformComponent>())
				targetPos = parentTransform->GetWorldPosition();
		}

		if (m_bCursorLocked)
		{
			float thetaDeltaDeg = xDelta * m_Sensitivity;
			float thetaDeltaRad = XMConvertToRadians(thetaDeltaDeg);
			float phiDeltaDeg = yDelta * m_Sensitivity;
			float phiDeltaRad = XMConvertToRadians(phiDeltaDeg);

			m_Phi += phiDeltaRad;
			m_Theta += thetaDeltaRad;

			m_Phi = std::clamp(m_Phi, 0.01f, XM_PI - 0.01f);

			if (m_Theta > XM_2PI)
				m_Theta -= XM_2PI;
			else if (m_Theta < 0)
				m_Theta += XM_2PI;
		}

		Vector3 localOffset = {
			m_Radius * sinf(m_Phi) * cosf(m_Theta),
			m_Radius * cosf(m_Phi),
			m_Radius * sinf(m_Phi) * sinf(m_Theta)
		};

		//Vector3 rotatedOffset = Vector3::Transform(localOffset, parentQuat);

		Vector3 camWorldPos = targetPos + localOffset;

		Vector3 forward = (targetPos - camWorldPos);
		if (forward.LengthSquared() > 0.00001f)
			forward.Normalize();
		else
			forward = Vector3::UnitZ;

		Vector3 right = Vector3::Up.Cross(forward);
		if(right.LengthSquared() < 0.00001f)
		{
			right = Vector3::Right;
		}
		right.Normalize();

		Vector3 up = forward.Cross(right);
		up.Normalize();

		Matrix rotationMatrix(
			right.x  , right.y  , right.z  , 0,
			up.x     , up.y     , up.z     , 0,
			forward.x, forward.y, forward.z, 0,
			0        , 0        , 0        , 1
		);

		auto newQuat = Quaternion::CreateFromRotationMatrix(rotationMatrix);

		transform->SetWorldPositionQuat(camWorldPos, newQuat);
	}

	void OnMousePressed(Blainn::MouseButton mouseButton)
	{
		if (!m_bCursorLocked && mouseButton == Blainn::MouseButton::Left)
		{
			Blainn::Input::SetCursorMode(Blainn::CursorMode::Locked);
			m_bCursorLocked = true;
		}
	}

private:
	DelegateHandle m_MouseMovedHandle;
	DelegateHandle m_MouseButtonPressedHandle;
	DelegateHandle m_KeyPressedHandle;

	float m_Phi = DirectX::XM_PIDIV2;
	float m_Theta = 0.f;

	float m_Radius = 10.f;
	float m_Sensitivity = 0.1f;
	float m_Speed = 10.f;

	bool m_bCursorLocked = false;
};
