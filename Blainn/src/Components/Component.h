#pragma once

#include "ComponentManager.h"

#include <memory>

namespace Blainn
{
	class GameObject;
	class GameTimer;

	class ComponentBase {
	public:
		virtual ~ComponentBase() = default;
		virtual void OnAttach() {};
		virtual void OnDestroy() {};
		virtual void OnInit() {};
		virtual void OnBegin() {};
		virtual void OnUpdate(const GameTimer& gt) {};
	};

	template<typename Derived>
	class Component : public std::enable_shared_from_this<Derived>, public ComponentBase
	{
		friend class GameObject;
	protected:
		Component(std::shared_ptr<GameObject> owner)
			: m_OwningObject(owner)
		{ OnInit(); }
	public:
		virtual ~Component() = default;
		
		virtual void OnInit() {}
		virtual void OnAttach();
		virtual void OnBegin() {}
		virtual void OnUpdate(const GameTimer& gt) {}
		virtual void OnDestroy();

		std::shared_ptr<GameObject> GetOwner() const { return m_OwningObject.lock(); }

	protected:
		std::weak_ptr<GameObject> m_OwningObject;
	};

	template<typename Derived>
	inline void Component<Derived>::OnAttach()
	{
		auto derivedPtr = std::static_pointer_cast<Derived>(this->shared_from_this());
		ComponentManager::Get().RegisterComponent(derivedPtr);
	}

	template<typename Derived>
	inline void Component<Derived>::OnDestroy()
	{
		auto derivedPtr = std::static_pointer_cast<Derived>(this->shared_from_this());
		ComponentManager::Get().UnregisterComponent(derivedPtr);
	}

}
