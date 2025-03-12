#include "SolarAppLayer.h"

#include <string>

#include "SimpleMath.h"

#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/CharacterComponents/PlayerInputComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Core/Application.h"
#include "Core/Events/Event.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"
#include "Scene/Actor.h"

#include "Planet.h"
#include "Player.h"
#include "WeirdOne.h"

namespace solar
{

	SolarAppLayer::SolarAppLayer()
		: m_Scene(Blainn::Application::Get().GetScene())
	{
	}

	void SolarAppLayer::OnAttach()
	{
		Super::OnAttach();

		SetupInput();

		auto sun = m_Scene->QueueGameObject<Planet>("sun", 0.f);
		m_CelestialBodies.push_back(sun);
		auto mesh = Blainn::DXModel::ColoredCube(6.f, DirectX::SimpleMath::Color{ 0.9f, 0.9f, 0.2f, 1.f });
		sun->AddComponent<Blainn::StaticMeshComponent>(mesh);
		auto* transform = sun->GetComponent<Blainn::TransformComponent>();
		transform->SetLocalPosition({ 0.f, 0.f, 0.f });

		auto planet1 = sun->AddChild<Planet>("planet 1", 8.f, 0.f, 2.f, Planet::DIRCounterClockwise, 10.f);
		m_CelestialBodies.push_back(planet1);
		planet1->AddComponent<Blainn::StaticMeshComponent>(Blainn::DXModel::ColoredCube(1.f, DirectX::SimpleMath::Color{ 0.1f, 0.2f, 0.3f, 1.f }));

		auto planet2 = sun->AddChild<Planet>("planet 2", 13.f, 40.f, 1.9f, Planet::DIRCounterClockwise, 10.f);
		m_CelestialBodies.push_back(planet2);
		planet2->AddComponent<Blainn::StaticMeshComponent>(Blainn::DXModel::ColoredCube(1.7f, DirectX::SimpleMath::Color{ 0.1f, 0.2f, 0.3f, 1.f }));

		auto planet3 = sun->AddChild<Planet>("planet 3", 19.f, 15.f, 3.8f, Planet::DIRClockwise, 70.f);
		m_CelestialBodies.push_back(planet3);
		planet3->AddComponent<Blainn::StaticMeshComponent>(Blainn::DXModel::ColoredCube(.6f, DirectX::SimpleMath::Color{ 0.1f, 0.2f, 0.3f, 1.f }));
		transform = planet3->GetComponent<Blainn::TransformComponent>();

		auto planet4 = sun->AddChild<Planet>("planet 4", 25.f, 170.f, 8.f, Planet::DIRCounterClockwise, 10.f);
		m_CelestialBodies.push_back(planet4);
		planet4->AddComponent<Blainn::StaticMeshComponent>(Blainn::DXModel::ColoredCube(2.f, DirectX::SimpleMath::Color{ 0.1f, 0.2f, 0.3f, 1.f }));
		auto moon4_1 = planet4->AddChild<Planet>("moon 4-1", 4.f, 15.f, 3.8f, Planet::DIRClockwise, 70.f);
		moon4_1->AddComponent<Blainn::StaticMeshComponent>(Blainn::DXModel::ColoredCube(.7f, DirectX::SimpleMath::Color{ 0.5f, 0.2f, 0.3f, 1.f }));

		m_CustomObject = m_Scene->QueueGameObject<Blainn::GameObject>();
		m_CustomObject->AddComponent<Blainn::TransformComponent>();
		m_CustomObject->AddComponent<Blainn::StaticMeshComponent>(Blainn::DXModel::ColoredCube(.5f));

		m_Player = m_Scene->QueueGameObject<Player>();
		m_Player->GetComponent<Blainn::TransformComponent>()->SetWorldPosition({0.f, 0.f, -5.f});
		m_Scene->SetMainCamera(m_Player->GetComponent<Blainn::CameraComponent>());

	}

	void SolarAppLayer::OnUpdate(const Blainn::GameTimer& gt)
	{
		//auto* transform = m_CustomObject->GetComponent<Blainn::TransformComponent>();
		//auto curPos = transform->GetWorldPosition();
		//curPos.x += gt.DeltaTime() * 1.f;
		////curPos.y += gt.DeltaTime() * 1.f;
		//auto curRot = transform->GetLocalYawPitchRoll();
		//curRot.y += 20.f * gt.DeltaTime();
		//transform->SetLocalPositionYawPitchRoll(curPos, curRot);
	//	transform->SetLocalYawPitchRoll(curRot);
	}

	void SolarAppLayer::OnEvent(Blainn::Event& event)
	{

	}

	void SolarAppLayer::OnDetach()
	{
	}

	void SolarAppLayer::OnKeyPressed(Blainn::KeyCode code)
	{
		if (code == Blainn::KeyCode::Num1)
		{
			CycleThroughBodies(-1);
			if(m_Player->GetInputMode() != Player::InputMode::Orbital)
				m_Player->ChangeInputModeToOrbital();
		}
		else if (code == Blainn::KeyCode::Num2)
		{
			CycleThroughBodies(1);
			if(m_Player->GetInputMode() != Player::InputMode::Orbital)
				m_Player->ChangeInputModeToOrbital();
		}
		else if (code == Blainn::KeyCode::Num3)
		{
			if(m_Player->GetInputMode() != Player::InputMode::Orbital)
				m_Player->ChangeInputModeToOrbital();
		}
		else if (code == Blainn::KeyCode::Num4)
		{
			m_Player->AttachTo(m_CustomObject);
			if(m_Player->GetInputMode() != Player::InputMode::Orbital)
				m_Player->ChangeInputModeToOrbital();
		}
		else if (code == Blainn::KeyCode::Num0)
		{
			DetachCamera();
			if(m_Player->GetInputMode() != Player::InputMode::FPS)
				m_Player->ChangeInputModeToFPS();
		}
	}

	void SolarAppLayer::CycleThroughBodies(int dir)
	{
		currentSelection = (currentSelection + dir) % m_CelestialBodies.size();
		m_Player->AttachTo(m_CelestialBodies[currentSelection]);
	}

	void SolarAppLayer::DetachCamera()
	{
		m_Player->AttachTo(nullptr);
	}

	void SolarAppLayer::SetupInput()
	{
		Blainn::Input::OnKeyPressedDelegate.AddRaw(this, &solar::SolarAppLayer::OnKeyPressed);
	}

}
