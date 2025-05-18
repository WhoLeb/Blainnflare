#pragma once

#include <memory>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

// has_static_Create
// returns true if there is at least one static member function called Create
// otherwise retuns false
template<typename, typename = std::void_t<>>
struct has_static_Create : std::false_type {};

template<typename T>
struct has_static_Create<T, std::void_t<decltype(&T::Create)>> : std::true_type {};

namespace Blainn
{
	class ComponentBase;
	class GameObject;

	class ComponentSetBase {
	public:
		virtual ~ComponentSetBase() = default;
	};

	template<typename T>
	class ComponentSet : public ComponentSetBase {
	public:
		std::unordered_set<std::shared_ptr<T>> Components;
	};

	class ComponentManager
	{
	public:
		static ComponentManager& Get()
		{
			static ComponentManager instance;
			return instance;
		}
		
		template<typename T, typename... Args>
		std::shared_ptr<T> MakeComponent(std::shared_ptr<GameObject> owner, Args&&... args)
		{
			if constexpr (has_static_Create<T>::value)
			{
				return T::Create(owner, std::forward<Args>(args)...);
			}
			else {
				return std::make_shared<T>(owner, std::forward<Args>(args)...);
			}
		}

		template<typename T>
		void RegisterComponent(std::shared_ptr<T> component)
		{
			static_assert(std::is_base_of<ComponentBase, T>::value, "Component must be derived from component");
			GetOrCreateComponentSet<T>().Components.insert(component);
		}

		template<typename T>
		void UnregisterComponent(std::shared_ptr<T> component)
		{
			static_assert(std::is_base_of<ComponentBase, T>::value, "Component must be derived from component");
			auto* set = GetComponentSet<T>();
			if (set) set->Components.erase(component);
		}

		template<typename T>
		const std::unordered_set<std::shared_ptr<T>>& GetComponents() const
		{
			static_assert(std::is_base_of<ComponentBase, T>::value, "Component must be derived from component");

			const auto* set = GetComponentSet<T>();

			return set ? set->Components : GetEmptySet<T>();
		}

	private:
		template<typename T>
		ComponentSet<T>& GetOrCreateComponentSet()
		{
			std::type_index index = typeid(T);

			auto it = m_ComponentMap.find(index);
			if (it == m_ComponentMap.end())
			{
				auto newSet = std::make_shared<ComponentSet<T>>();
				m_ComponentMap[index] = newSet;
				return *newSet;
			}

			return *static_cast<ComponentSet<T>*>(it->second.get());
		}

		template<typename T>
		ComponentSet<T>* GetComponentSet() const
		{
			std::type_index index = typeid(T);
			auto it = m_ComponentMap.find(index);
			return (it != m_ComponentMap.end())
				? static_cast<ComponentSet<T>*>(it->second.get())
				: nullptr;
		}

		template<typename T>
		static const std::unordered_set<std::shared_ptr<T>>& GetEmptySet() {
			static const std::unordered_set<std::shared_ptr<T>> emptySet;
			return emptySet;
		}

	private:
		ComponentManager() = default;
		~ComponentManager() = default;
		ComponentManager(const ComponentManager&) = delete;
		ComponentManager& operator=(const ComponentManager&) = delete;
		ComponentManager(const ComponentManager&&) = delete;
		ComponentManager& operator=(const ComponentManager&&) = delete;

		std::unordered_map<std::type_index, std::shared_ptr<ComponentSetBase>> m_ComponentMap;
	};
}
