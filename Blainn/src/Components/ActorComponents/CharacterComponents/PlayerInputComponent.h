#pragma once

#include "Components/ActorComponents/TransformComponent.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"
#include "InputComponent.h"

#include <iostream>

namespace Blainn
{
	class PlayerInputComponent : public InputComponent
	{
		using Super = InputComponent;
	public:
		PlayerInputComponent(float speed = 5.f, float sensitivity = .1f)
			: Super(), m_Speed(speed), m_Sensitivity(sensitivity)
		{}

		void HandleInput(const GameTimer& gt)
		{
			using namespace DirectX;
			using namespace DirectX::SimpleMath;

			auto* owner = GetOwner();
			if (!owner) return;

			auto* transform = owner->GetComponent<TransformComponent>();
			if (!transform) return;

			if (Input::IsMouseButtonHeld(MouseButton::Right))
			{
				Input::SetCursorMode(CursorMode::Locked);

				auto [mouseDX, mouseDY] = Input::GetMouseDelta();

				float yawDeltaRad = XMConvertToRadians(mouseDX * m_Sensitivity);
				float pitchDeltaRad  = XMConvertToRadians(mouseDY * m_Sensitivity);

				auto currentQuat = transform->GetLocalQuat();

				Quaternion yawQuat = Quaternion::CreateFromAxisAngle({ 0.f, 1.f, 0.f }, yawDeltaRad);
				Quaternion pitchQuat = Quaternion::CreateFromAxisAngle({1.f, 0.f, 0.f}, pitchDeltaRad);

				Quaternion finalQuat = pitchQuat * currentQuat * yawQuat;

				finalQuat.Normalize();
				transform->SetWorldQuat(finalQuat);

				DirectX::SimpleMath::Vector3 movement = { 0, 0, 0 };

				if (Input::IsKeyHeld(KeyCode::W)) movement.z += 1.0f;
				if (Input::IsKeyHeld(KeyCode::S)) movement.z -= 1.0f;
				if (Input::IsKeyHeld(KeyCode::A)) movement.x -= 1.0f;
				if (Input::IsKeyHeld(KeyCode::D)) movement.x += 1.0f;
				if (Input::IsKeyHeld(KeyCode::Q)) movement.y -= 1.0f;
				if (Input::IsKeyHeld(KeyCode::E)) movement.y += 1.0f;

				if (movement.LengthSquared() > 0)
				{
					auto forward = transform->GetWorldForwardVector();
					auto right = transform->GetWorldRightVector();
					auto up = Vector3{ 0, 1, 0 };
					
					auto moveDir = right * movement.x + up * movement.y + forward * movement.z;
					moveDir.Normalize();

					auto prevPos = transform->GetWorldPosition();

					transform->SetWorldPosition(prevPos + moveDir * m_Speed * gt.DeltaTime());
				}

			}
			else
			{
				// Unlock cursor when not rotating
				Input::SetCursorMode(CursorMode::Normal);
			}
		}

	private:
		float m_Speed;
		float m_Sensitivity;
	};
}



