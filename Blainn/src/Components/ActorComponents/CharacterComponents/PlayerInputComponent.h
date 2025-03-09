#pragma once

#include "Components/ActorComponents/TransformComponent.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"
#include "InputComponent.h"

namespace Blainn
{
	class PlayerInputComponent : public InputComponent
	{
		using Super = InputComponent;
	public:
		PlayerInputComponent(float speed = 5.f, float sensitivity = 0.1f)
			: Super(), m_Speed(speed), m_Sensitivity(sensitivity)
		{}

		void HandleInput(const GameTimer& gt)
		{
			auto* owner = GetOwner();
			if (!owner) return;

			auto* transform = owner->GetComponent<TransformComponent>();
			if (!transform) return;

			if (Input::IsMouseButtonPressed(MouseButton::Right))
			{
				Input::SetCursorMode(CursorMode::Locked);

				DirectX::SimpleMath::Vector3 movement = { 0, 0, 0 };

				if (Input::IsKeyPressed(KeyCode::W)) movement.z += 1.0f;
				if (Input::IsKeyPressed(KeyCode::S)) movement.z -= 1.0f;
				if (Input::IsKeyPressed(KeyCode::A)) movement.x -= 1.0f;
				if (Input::IsKeyPressed(KeyCode::D)) movement.x += 1.0f;

				movement.Normalize();

				// Apply movement
				transform->SetPosition({
					transform->GetWorldPosition().x + movement.x * m_Speed * gt.DeltaTime(),
					transform->GetWorldPosition().y,
					transform->GetWorldPosition().z + movement.z * m_Speed * gt.DeltaTime()
					});

				auto [mouseDX, mouseDY] = Input::GetMouseDelta();

				float yaw = mouseDX * m_Sensitivity;
				float pitch = mouseDY * m_Sensitivity;

				transform->SetRotation({
					transform->GetWorldRotation().x + pitch,
					transform->GetWorldRotation().y + yaw,
					transform->GetWorldRotation().z
					});
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



