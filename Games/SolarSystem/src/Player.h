#pragma once

#include "Scene/Actor.h"
#include "Components/ActorComponents/CharacterComponents/InputComponent.h"
#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"

#include <string>

namespace solar
{
	class Planet;
	class Player : public Blainn::Actor
	{
		using Super = Blainn::Actor;
	public:
		enum class InputMode
		{
			FPS,
			Orbital
		} m_InputMode;

		Player(const std::string& name = "Player");

		void OnUpdate(const Blainn::GameTimer& gt) override;

		void ChangeInputModeToOrbital();
		void ChangeInputModeToFPS();

		InputMode GetInputMode() const { return m_InputMode; }
	private:
		std::string m_Name;
		
		float m_Timer = 3.f;
		Planet* m_CurrentTarget;
	};
}
