#pragma once

#include "CollisionComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Core/GameObject.h"

#include "DirectXCollision.h"

namespace Blainn
{
	class SphereCollisionComponent : public CollisionComponent
	{
		using Super = CollisionComponent;
	public:
		SphereCollisionComponent(float radius)
		{
			m_BoundingSphere.Radius = radius;
		}

		void OnAttach() override
		{
			Super::OnAttach();
			if (!GetOwner()) return;
			auto transform = GetOwner()->GetComponent<TransformComponent>();
			if (!transform) return;
			m_BoundingSphere.Center = transform->GetWorldPosition();
		}

		void* GetBoundingShape() override { return static_cast<void*>(&m_BoundingSphere); }

		void OnUpdate(const GameTimer& gt) override
		{
			if (!GetOwner()) return;
			auto transform = GetOwner()->GetComponent<TransformComponent>();
			if (!transform) return;
			m_BoundingSphere.Center = transform->GetWorldPosition();
		}

		bool Intersects(std::shared_ptr<CollisionComponent> collider)
		{
			auto boundingSphere = static_cast<DirectX::BoundingSphere*>(collider->GetBoundingShape());
			return m_BoundingSphere.Intersects(*boundingSphere);
		}

		float GetRadius()
		{
			return m_BoundingSphere.Radius;
		}

		void UpdateRadius(float newRadius)
		{
			m_BoundingSphere.Radius = newRadius;
		}

	private:
		DirectX::BoundingSphere m_BoundingSphere;

	};
}
