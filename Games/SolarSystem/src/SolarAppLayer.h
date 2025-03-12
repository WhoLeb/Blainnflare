#pragma once

#include "windows.h"

#include "Core/Layer.h"

#include "Core/KeyCodes.h"
#include "Scene/Actor.h"
#include "Scene/Scene.h"

namespace Blainn
{
	class GameObject;
}

namespace solar
{
	class Planet;
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
		void OnKeyPressed(Blainn::KeyCode code);
		void CycleThroughBodies(int dir);
		void DetachCamera();

		void SetupInput();

	private:
		std::shared_ptr<Blainn::Scene> m_Scene;

		std::vector<std::shared_ptr<Blainn::Actor>> m_CelestialBodies;
		std::shared_ptr<Blainn::GameObject> m_CustomObject;
		int currentSelection = 0;

		std::shared_ptr<Player> m_Player;
	};
}
