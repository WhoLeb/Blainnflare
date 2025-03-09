#pragma once

#include "windows.h"

#include "Core/Layer.h"

#include "Scene/Scene.h"

namespace solar
{
	class Player;
	class SolarAppLayer : public Blainn::Layer
	{
		using Super = Blainn::Layer;
	public:
		SolarAppLayer();

		void OnAttach() override;
		void OnUpdate(const Blainn::GameTimer& gt) override;
		void OnEvent(Blainn::Event& event) override;
		void OnDetach() override;

	private:
		std::shared_ptr<Blainn::Scene> m_Scene;

		std::shared_ptr<Player> m_Player;
	};
}
