#pragma once

#include "Components/ActorComponents/TransformComponent.h"
#include "Components/Component.h"
#include "Core/Camera.h"
#include "Core/GameObject.h"

namespace Blainn
{
	class CameraComponent : public Component<CameraComponent>
	{
		using Super = Component;
	public:
		explicit CameraComponent(
			float fov = 60.f,
			float aspect = 16.f / 9.f,
			float nearPlane = 0.1f,
			float farPlane = 1000.f
		) : Super(), m_Camera(fov, aspect, nearPlane, farPlane)
		{}
		explicit CameraComponent(
			int width, int height,
			float fov = 60.f,
			float nearPlane = 0.1f,
			float farPlane = 1000.f
		) : Super(), m_Camera(fov, width, height, nearPlane, farPlane)
		{}

		void OnUpdate(const GameTimer& gt) override
		{
			Super::OnUpdate(gt);

			auto owner = GetOwner();
			if (!owner) return;

			auto transform = owner->GetComponent<TransformComponent>();
			if (!transform) return;

			m_Camera.SetPositionAndQuaternion(transform->GetWorldPosition(), transform->GetWorldQuat());
		}

		Camera& GetCamera() { return m_Camera; }
	private:
		Camera m_Camera;
	};
}
