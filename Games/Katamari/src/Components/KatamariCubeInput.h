#pragma once

#include "Components/Component.h"

#include "Core/GameObject.h"
#include "Components/ActorComponents/CharacterComponents/InputComponent.h"
#include "Components/ActorComponents/TransformComponent.h"

#include "SimpleMath.h"

#include <algorithm>

class KatamariCubeInput : public Blainn::InputComponent
{
public:
	KatamariCubeInput() = default;

	void HandleInput(const Blainn::GameTimer& gt) override
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		using namespace Blainn;

		auto owner = GetOwner();
		if (!owner) return;

		auto transform = owner->GetComponent<Blainn::TransformComponent>();
		if (!transform) return;

		auto parent = owner->GetParent();
		if (!parent) return;

		auto parentTransform = parent->GetComponent<Blainn::TransformComponent>();
		if (!parentTransform) return;

		if (Input::GetCursorMode() != CursorMode::Locked)
			return;

		if (Input::IsKeyHeld(KeyCode::W))
		{
			auto right = parent->GetChildren()[0]->GetComponent<TransformComponent>()->GetWorldRightVector();
			auto quat = transform->GetWorldQuat();

			auto newQuat = quat * Quaternion::CreateFromAxisAngle(right, m_RotationSpeed * gt.DeltaTime());
			transform->SetWorldQuat(newQuat);
		}
		if (Input::IsKeyHeld(KeyCode::S))
		{
			auto right = parent->GetChildren()[0]->GetComponent<TransformComponent>()->GetWorldRightVector();
			auto quat = transform->GetWorldQuat();

			auto newQuat = quat * Quaternion::CreateFromAxisAngle(-right, m_RotationSpeed * gt.DeltaTime());
			transform->SetWorldQuat(newQuat);
		}
	}
private:
	float m_RotationSpeed = DirectX::XM_PI;
};
