#pragma once

#include "Components/ComponentManager.h"
#include "Components/Component.h"
#include "Core/Input.h"

namespace Blainn
{
	class InputComponent : public Component<InputComponent>
	{
		using Super = Component<InputComponent>;
	protected:
		InputComponent() : Super()
		{
		}
	public:
		virtual ~InputComponent()
		{
		}


		virtual void HandleInput(const GameTimer& gt) {}

		void OnAttach() override
		{
			Super::OnAttach();
		}

		void OnUpdate(const GameTimer& gt) override
		{
			HandleInput(gt);
		}

	protected:

	};
}
