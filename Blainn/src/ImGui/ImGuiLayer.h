#pragma once

#include "Core/Layer.h"


namespace Blainn
{
	class GameTimer;
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(const GameTimer& gt) override;
		void OnEvent(Event& event) override;
	private:

	};

}
