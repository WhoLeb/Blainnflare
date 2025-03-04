#pragma once

#include "Core/GameTimer.h"
#include "Core/UUID.h"

namespace Blainn
{
	class GameObject
	{
	public:
		GameObject()
		{}
		virtual ~GameObject() noexcept {}

		virtual void OnInit() {}
		virtual void OnBegin() {}
		virtual void OnUpdate(const GameTimer& gt) {}
		virtual void OnDestroy() {}

	protected:
		UUID m_UUID{};
	};
}

