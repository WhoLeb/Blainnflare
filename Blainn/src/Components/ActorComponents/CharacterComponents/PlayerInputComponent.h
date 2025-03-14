#pragma once

#include <algorithm>
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
		PlayerInputComponent(float speed = 5.f, float lowSpeed = 5.f, float highSpeed = 10.f, float sensitivity = .1f)
			: Super(), m_Speed(speed), m_LowSpeed(lowSpeed), m_HighSpeed(highSpeed), m_Sensitivity(sensitivity)
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

				auto [totalYaw, totalPitch, totalRoll] = transform->GetWorldYawPitchRoll();
				totalYaw += mouseDX * m_Sensitivity;
				totalPitch += mouseDY * m_Sensitivity;

				if (totalYaw > 360.f)
					totalYaw -= 360.f;
				else if (totalYaw < 0.)
					totalYaw += 360.f;
					
				totalPitch = std::clamp(totalPitch, -89.0f, 89.0f);
				

				float yawRad = XMConvertToRadians(totalYaw);
				float pitchRad = XMConvertToRadians(totalPitch);

				Quaternion yawQuat = Quaternion::CreateFromAxisAngle({ 0.0f, 1.0f, 0.0f }, yawRad);
				Quaternion pitchQuat = Quaternion::CreateFromAxisAngle({ 1.0f, 0.0f, 0.0f }, pitchRad);

				Quaternion finalQuat = pitchQuat * yawQuat;
				finalQuat.Normalize();

				transform->SetWorldQuat(finalQuat);

				DirectX::SimpleMath::Vector3 movement = { 0, 0, 0 };

				if (Input::IsKeyHeld(KeyCode::W)) movement.z += 1.0f;
				if (Input::IsKeyHeld(KeyCode::S)) movement.z -= 1.0f;
				if (Input::IsKeyHeld(KeyCode::A)) movement.x -= 1.0f;
				if (Input::IsKeyHeld(KeyCode::D)) movement.x += 1.0f;
				if (Input::IsKeyHeld(KeyCode::Q)) movement.y -= 1.0f;
				if (Input::IsKeyHeld(KeyCode::E)) movement.y += 1.0f;
				float speed;
				if (Input::IsKeyHeld(KeyCode::Shift))
					speed = m_HighSpeed;
				else
					speed = m_LowSpeed;

				if (movement.LengthSquared() > 0)
				{
					auto forward = transform->GetWorldForwardVector();
					auto right = transform->GetWorldRightVector();
					auto up = Vector3{ 0, 1, 0 };

					auto moveDir = right * movement.x + up * movement.y + forward * movement.z;
					moveDir.Normalize();

					auto prevPos = transform->GetWorldPosition();

					transform->SetWorldPosition(prevPos + moveDir * speed * gt.DeltaTime());
				}

			}
			else
			{
				Input::SetCursorMode(CursorMode::Normal);
			}
		}

	private:
		float m_Speed;
		float m_Sensitivity;

		const float m_LowSpeed;
		const float m_HighSpeed;
	};
}



