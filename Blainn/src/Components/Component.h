#pragma once

namespace Blainn
{
	class Component
	{
	public:
		Component() {}
		virtual ~Component() { }

		virtual Component* GetParent() const { return m_Parent.get(); }

		virtual void Destroy() {
			for (auto& child : m_Children)
				child->Destroy();
		}

	protected:
		std::unique_ptr<Component> m_Parent;

		std::vector<std::shared_ptr<Component>> m_Children;
	};

	template <typename T, typename... Args>
	std::shared_ptr<T> CreateComponent(Args... );
}
