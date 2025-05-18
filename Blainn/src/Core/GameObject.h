#pragma once

#include "Core/GameTimer.h"
#include "Core/UUID.h"
#include "Components/Component.h"
#include "Scene/Scene.h"

namespace Blainn
{

	class GameObject : public std::enable_shared_from_this<GameObject>
	{
		friend class Scene;
	public:
		GameObject() { OnInit(); }
		virtual ~GameObject() noexcept {}

		virtual void OnInit() {}
		virtual void OnBegin() {}

		virtual void OnAttach()
		{
			for (auto& comp : m_Components)
				comp->OnAttach();
			for (auto& child : m_Children)
				child->OnAttach();
		}

		virtual void OnUpdate(const GameTimer& gt)
		{
			for (auto& component : m_Components)
				component->OnUpdate(gt);
			for (auto& child : m_Children)
				child->OnUpdate(gt);
		}

		virtual void OnDestroy() {}

		const std::vector<std::shared_ptr<ComponentBase>>& GetComponents() const
		{
			return m_Components;
		}

		template<typename T>
		std::vector<std::shared_ptr<T>> GetComponents() const
		{
			static_assert(std::is_base_of<ComponentBase, T>::value, "Component must be derived from component");
			std::vector<std::shared_ptr<T>> foundComponents;

			for (const auto& comp : m_Components)
				if (std::shared_ptr<T> castedComp = std::dynamic_pointer_cast<T>(comp))
					foundComponents.push_back(castedComp);

			return foundComponents;
		}


		template<typename T>
		std::shared_ptr<T> GetComponent() const
		{
			static_assert(std::is_base_of<ComponentBase, T>::value, "Component must be derived from component");
			for (const auto& comp : m_Components)
				if (std::shared_ptr<T> castedComp = std::dynamic_pointer_cast<T>(comp))
					return castedComp;
			return nullptr;
		}


		template<typename T, typename... Args>
		std::shared_ptr<T> AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must be a component");
			auto component = ComponentManager::Get().MakeComponent<T>(shared_from_this(), std::forward<Args>(args)...);
			component->OnAttach();
			m_Components.push_back(component);
			return component;
		}


		template<typename T>
		void RemoveAllComponents()
		{
			auto it = std::remove_if(m_Components.begin(), m_Components.end(),
				[](const std::shared_ptr<ComponentBase>& comp)
				{
					return dynamic_cast<T*>(comp.get()) != nullptr;
				});

			for (auto i = it; i != m_Components.end(); i++)
				(*i)->OnDestroy;

			m_Components.erase(it, m_Components.end());
		}


		void RemoveComponent(std::shared_ptr<ComponentBase> component)
		{
			auto it = std::find_if(m_Components.begin(), m_Components.end(),
				[&component](const std::shared_ptr<ComponentBase>& comp)
				{
					return comp == component;
				});
			
			if (it != m_Components.end())
			{
				(*it)->OnDestroy();
				m_Components.erase(it);
			}
		}


		template<typename T, typename... Args>
		std::shared_ptr<T> AddChild(Args&&... args)
		{
			static_assert(std::is_base_of<GameObject, T>::value, "The child object must be a game object");

			auto child = std::make_shared<T>(std::forward<Args>(args)...);
			child->m_Parent = shared_from_this();
			m_Children.push_back(child);

			if (m_ParentScene)
			{
				child->m_ParentScene = m_ParentScene;
				m_ParentScene->QueueGameObject(child);
			}
			return child;
		}


		void RemoveChild(std::shared_ptr<GameObject> child)
		{
			auto it = std::find(m_Children.begin(), m_Children.end(), child);
			if (it != m_Children.end())
			{
				child->m_Parent.reset();
				if (m_ParentScene)
				{
					child->m_ParentScene = nullptr;
					m_ParentScene->RemoveGameObject(child);
				}
				m_Children.erase(it);
			}
		}


		void AddChild(std::shared_ptr<GameObject> child)
		{
			auto prevParent = child->m_Parent.lock();
			if (prevParent)
			{
				auto it = std::find(prevParent->m_Children.begin(), prevParent->m_Children.end(), child);
				if (it != prevParent->m_Children.end())
					prevParent->m_Children.erase(it);

				child->m_Parent.reset();
			}

			child->m_Parent = shared_from_this();

			child->OnAttach();

			m_Children.push_back(child);
		}


		void AttachTo(std::shared_ptr<GameObject> newParent)
		{
			if (newParent == m_Parent.lock())
				return;
			if(newParent)
				newParent->AddChild(shared_from_this());
			else
			{
				auto prevParent = m_Parent.lock();
				if (prevParent)
				{
					auto it = std::find(prevParent->m_Children.begin(), prevParent->m_Children.end(), shared_from_this());
					if (it != prevParent->m_Children.end())
						prevParent->m_Children.erase(it);

					m_Parent.reset();
				}
			}
		}


		const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return m_Children; }
		UUID GetUUID() const { return m_UUID; }
		std::shared_ptr<GameObject> GetParent() const { return m_Parent.lock(); }

	protected:
		UUID m_UUID{};

		std::weak_ptr<GameObject> m_Parent;
		std::vector<std::shared_ptr<GameObject>> m_Children;
		std::vector<std::shared_ptr<ComponentBase>> m_Components;

		Scene* m_ParentScene = nullptr;
	};
}

