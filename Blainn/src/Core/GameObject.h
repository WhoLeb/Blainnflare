#pragma once

#include "Core/GameTimer.h"
#include "Core/UUID.h"
#include "Components/Component.h"

namespace Blainn
{
	class GameObject : public std::enable_shared_from_this<GameObject>
	{
		friend class Scene;
	protected:
		GameObject() { OnInit(); }
	public:
		virtual ~GameObject() noexcept {}

		UUID GetUUID() const { return m_UUID; }

		virtual void OnInit() {}
		virtual void OnBegin() {}
		virtual void OnUpdate(const GameTimer& gt)
		{
			for (auto& component : m_Components)
				component->OnUpdate(gt);
			for (auto& child : m_Children)
				child->OnUpdate(gt);
		}
		virtual void OnDestroy() {}

		template<typename T>
		std::vector<T*> GetComponents() const
		{
			std::vector<T*> foundComponents;

			for (const auto& comp : m_Components)
				if (T* castedComp = dynamic_cast<T*>(comp.get()))
					foundComponents.push_back(castedComp);

			return foundComponents;
		}
		template<typename T>
		T* GetComponent() const
		{
			for (const auto& comp : m_Components)
				if (T* castedComp = dynamic_cast<T*>(comp.get()))
					return castedComp;
			return nullptr;
		}

		template<typename T, typename... Args>
		T* AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must be a component");
			auto component = std::make_unique<T>(std::forward<Args>(args)...);
			component->m_OwningObject = this;
			component->OnBegin();
			m_Components.emplace_back(component);
			return component;
		}
		template<typename T>
		void RemoveAllComponents()
		{
			auto it = std::remove_if(m_Components.begin(), m_Components.end(),
					[](const std::unique_ptr<Component>& comp)
					{
						return dynamic_cast<T*>(comp->GetOwner()) != nullptr;
					}),
				m_Components.end();
			for (auto i = it; i < m_Components.end(); i++)
				i->OnDestroy;
			m_Components.erase(it, m_Components.end());
		}

		void RemoveComponent(Component* component)
		{
			auto it = std::find_if(m_Components.begin(), m_Components.end(),
				[component](const std::unique_ptr<Component>& comp)
				{
					return comp.get() == component;
				});
			
			if (it != m_Components.end())
			{
				component->OnDestroy();
				m_Components.erase(it);
			}
		}

		void AddChild(std::shared_ptr<GameObject> child)
		{
			child->m_Parent = shared_from_this();
			m_Children.emplace_back(std::move(child));
			child->OnBegin();
		}
		void RemoveChild(std::shared_ptr<GameObject> child)
		{
			auto it = std::find(m_Children.begin(), m_Children.end(), child);
			if (it != m_Children.end())
			{
				child->OnDestroy();
				child->m_Parent.reset();
				m_Children.erase(it);
			}
		}

	protected:
		UUID m_UUID{};

		std::weak_ptr<GameObject> m_Parent;
		std::vector<std::shared_ptr<GameObject>> m_Children;
		
		std::vector<std::unique_ptr<Component>> m_Components;
	};
}

