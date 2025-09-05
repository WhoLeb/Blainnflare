#pragma once

#include "Components/Component.h"
#include "Components/ComponentManager.h"

#include <functional>

namespace Blainn
{
	class GameObject;

	class CollisionComponent : public Component<CollisionComponent>
	{
		using CollisionCallbackFn = std::function<void(std::shared_ptr<CollisionComponent>)>;
		using Super = Component<CollisionComponent>;
	public:
		CollisionComponent(std::shared_ptr<GameObject> owner)
			: Super(owner)
		{}
		virtual ~CollisionComponent()
		{
		}

		void OnAttach()
		{
			Super::OnAttach();
		}

		virtual void* GetBoundingShape() = 0;
		
		virtual bool Intersects(std::shared_ptr<CollisionComponent> collider) = 0;

		void SetCollisionCallback(const CollisionCallbackFn& callback)
		{
			OnCollisionCallback = callback;
		}
		void OnCollision(std::shared_ptr<CollisionComponent> collidingObject)
		{
			if (OnCollisionCallback)
				OnCollisionCallback(collidingObject);
		}
	private:
		CollisionCallbackFn OnCollisionCallback;
	};
}
