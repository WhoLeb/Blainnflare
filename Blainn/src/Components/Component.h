#pragma once

namespace Blainn
{
	class GameObject;
	class GameTimer;

	class Component
	{
		friend class GameObject;
	protected:
		Component() { OnInit(); }
	public:
		virtual ~Component() { OnDestroy(); }
		
		virtual void OnInit() {}
		virtual void OnAttach() {};
		virtual void OnBegin() {}
		virtual void OnUpdate(const GameTimer& gt) {}
		virtual void OnDestroy() {}

		GameObject* GetOwner() const { return m_OwningObject; }

	protected:
		GameObject* m_OwningObject;
	};

}
