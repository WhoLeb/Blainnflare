#pragma once

#include "Components/ActorComponents/TransformComponent.h"
#include "Components/Component.h"
#include "Core/Camera.h"
#include "Core/GameObject.h"

namespace Blainn
{
	class CameraComponent : public Component
	{
		using Super = Component;
	protected:
		CameraComponent(
			float fov = 60.f,
			float aspect = 16.f / 9.f,
			float nearPlane = 0.1f,
			float farPlane = 1000.f
		) : Super(), m_Camera(fov, aspect, nearPlane, farPlane)
		{}
		CameraComponent(
			float fov = 60.f,
			int width = -1, int height = -1,
			float nearPlane = 0.1f,
			float farPlane = 1000.f
		) : Super(), m_Camera(fov, width, height, nearPlane, farPlane)
		{}

	public:
		void OnUpdate(const GameTimer& gt) override
		{
			auto* owner = GetOwner();
			if (!owner) return;

			auto* transform = owner->GetComponent<TransformComponent>();
			if (!transform) return;

			m_Camera.SetPositionAndRotation(transform->GetWorldPosition(), transform->GetWorldRotation());
		}

		Camera& GetCamera() { return m_Camera; }
	private:
		Camera m_Camera;
	};
}
