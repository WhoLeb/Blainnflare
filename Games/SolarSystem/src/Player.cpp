#include "Player.h"

#include "Core/GameTimer.h"
#include "Components/ActorComponents/CharacterComponents/OrbitalCameraController.h"
#include "Components/ActorComponents/CharacterComponents/PlayerInputComponent.h"

#include "Planet.h"

#include <iostream>


namespace solar
{
	Player::Player(const std::string& name)
		: m_Name(name)
	{
		ChangeInputModeToFPS();
		AddComponent<Blainn::CameraComponent>(800, 600);
	}

	void Player::OnUpdate(const Blainn::GameTimer& gt)
	{
		Super::OnUpdate(gt);

		//m_Timer -= gt.DeltaTime();
		//if (m_Timer <= 0.f)
		//{
		//	m_Timer = 3.f;

		//	std::cout << "===============================\n";
		//	std::cout << "Player local pos (X, Y, Z): ("
		//		<< transform->GetLocalPosition().x << ", "
		//		<< transform->GetLocalPosition().y << ", "
		//		<< transform->GetLocalPosition().z << ")\n"
		//		<< "Player world pos(X, Y, Z) : ("
		//		<< transform->GetWorldPosition().x << ", "
		//		<< transform->GetWorldPosition().y << ", "
		//		<< transform->GetWorldPosition().z << ")\n";
		//	if (GetParent())
		//	{
		//		if (auto* pt = GetParent()->GetComponent<Blainn::TransformComponent>())
		//			std::cout << "Parent local pos (X, Y, Z): ("
		//			<< pt->GetLocalPosition().x << ", "
		//			<< pt->GetLocalPosition().y << ", "
		//			<< pt->GetLocalPosition().z << ")\n"
		//			<< "Parent world pos(X, Y, Z) : ("
		//			<< pt->GetWorldPosition().x << ", "
		//			<< pt->GetWorldPosition().y << ", "
		//			<< pt->GetWorldPosition().z << ")\n"
		//			<< "Parent quaternion (W, X, Y, Z) : ("
		//			<< pt->GetWorldQuat().w << ", "
		//			<< pt->GetWorldQuat().x << ", "
		//			<< pt->GetWorldQuat().y << ", "
		//			<< pt->GetWorldQuat().z << ")\n";
		//		auto* targetPlanet = dynamic_cast<Planet*>(GetParent().get());
		//		if (targetPlanet)
		//			std::cout << "Current target - " << targetPlanet->GetName() << "\n";
		//	}

		//}
	}

	void Player::ChangeInputModeToOrbital()
	{
		RemoveComponent(GetComponent<Blainn::PlayerInputComponent>());
		AddComponent<Blainn::OrbitalInputComponent>();
		m_InputMode = InputMode::Orbital;
	}

	void Player::ChangeInputModeToFPS()
	{
		RemoveComponent(GetComponent<Blainn::OrbitalInputComponent>());
		AddComponent<Blainn::PlayerInputComponent>();
		m_InputMode = InputMode::FPS;
	}

}
