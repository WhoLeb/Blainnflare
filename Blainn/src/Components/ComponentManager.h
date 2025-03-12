#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace Blainn
{
	class ComponentManager
	{
	public:
		static ComponentManager& Get()
		{
			static ComponentManager instance;
			return instance;
		}

		template<typename T>
		void RegisterComponent(T* component)
		{
			static_assert(std::is_base_of<Component, T>::value, "Component must be derived from component");
			GetOrCreateComponentSet<T>().insert(component);
		}

		template<typename T>
		void UnregisterComponent(T* component)
		{
			static_assert(std::is_base_of<Component, T>::value, "Component must be derived from component");
			std::type_index index = typeid(T);

			auto it = m_ComponentMap.find(index);
			if (it != m_ComponentMap.end())
			{
				auto& set = *std::static_pointer_cast<std::unordered_set<T*>>(it->second);
				set.erase(component);
			}
		}

		template<typename T>
		const std::unordered_set<T*>& GetComponents() const
		{
			static_assert(std::is_base_of<Component, T>::value, "Component must be derived from component");
			std::type_index index = typeid(T);
			auto it = m_ComponentMap.find(index);
			if (it != m_ComponentMap.end())
			{
				return *std::static_pointer_cast<std::unordered_set<T*>>(it->second);
			}

			static const std::unordered_set<T*> emptySet{};
			return emptySet;
		}

	private:
		template<typename T>
		std::unordered_set<T*>& GetOrCreateComponentSet()
		{
			std::type_index index = typeid(T);

			auto it = m_ComponentMap.find(index);
			if (it == m_ComponentMap.end())
			{
				auto componentSet = std::make_shared<std::unordered_set<T*>>();
				m_ComponentMap[index] = componentSet;
				return *componentSet;
			}

			return *std::static_pointer_cast<std::unordered_set<T*>>(it->second);
		}

	private:
		ComponentManager() = default;
		~ComponentManager() = default;
		ComponentManager(const ComponentManager&) = delete;
		ComponentManager& operator=(const ComponentManager&) = delete;
		ComponentManager(const ComponentManager&&) = delete;
		ComponentManager& operator=(const ComponentManager&&) = delete;

		std::unordered_map<std::type_index, std::shared_ptr<void>> m_ComponentMap;
	};
}
