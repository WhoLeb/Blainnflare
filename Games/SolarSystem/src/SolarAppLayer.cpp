#include "SolarAppLayer.h"

#include <string>

#include "SimpleMath.h"

#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/CharacterComponents/PlayerInputComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Core/Application.h"
#include "Core/Events/Event.h"
#include "Core/GameTimer.h"

#include "Planet.h"
#include "Player.h"

namespace solar
{

	SolarAppLayer::SolarAppLayer()
		: m_Scene(Blainn::Application::Get().GetScene())
	{
	}

	void SolarAppLayer::OnAttach()
	{
		Super::OnAttach();

		auto planet1 = m_Scene->QueueGameObject<::solar::Planet>();
		planet1->AddComponent<Blainn::StaticMeshComponent>("Meshes/cube.obj");
		planet1->AddComponent<Blainn::TransformComponent>()->SetPosition({0.f, 0.f, 10.f});

		m_Player = m_Scene->QueueGameObject<Player>();
		m_Player->AddComponent<Blainn::TransformComponent>()->SetPosition({0.f, 0.f, -5.f});
		m_Player->AddComponent<Blainn::PlayerInputComponent>();
		m_Scene->SetMainCamera(m_Player->AddComponent<Blainn::CameraComponent>(800, 600));
	}

	void SolarAppLayer::OnUpdate(const Blainn::GameTimer& gt)
	{
	}

	void SolarAppLayer::OnEvent(Blainn::Event& event)
	{
	}

	void SolarAppLayer::OnDetach()
	{
	}

}
