#pragma once

#include "Components/Component.h"
#include "Core/Input.h"

namespace Blainn
{
	class InputComponent : public Component
	{
		using Super = Component;
	protected:
		InputComponent() : Super() {}
	public:
		virtual void HandleInput(const GameTimer& gt) {}

		void OnUpdate(const GameTimer& gt) override
		{
			HandleInput(gt);
		}
	};
}
