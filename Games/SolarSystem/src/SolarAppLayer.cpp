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

		m_Sun = m_Scene->QueueGameObject<Planet>();
		m_Sun->AddComponent<Blainn::StaticMeshComponent>("Meshes/wiener.gltf");
		auto* transform = m_Sun->AddComponent<Blainn::TransformComponent>();
		transform->SetWorldPosition({ 0.f, 0.f, 10.f });
		transform->SetWorldScale({ 3.f, 3.f, 3.f });


		auto planet1 = m_Sun->AddChild<Planet>();
		planet1->AddComponent<Blainn::StaticMeshComponent>("Meshes/wiener.gltf");
		planet1->AddComponent<Blainn::TransformComponent>();
		transform->SetLocalPosition({ 0.f, 0.f, 10.f });
		transform->SetWorldScale({ 3.f, 3.f, 3.f });

		m_Player = m_Scene->QueueGameObject<Player>();
		m_Player->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({0.f, 0.f, -5.f});
		m_Player->AddComponent<Blainn::PlayerInputComponent>();
		m_Scene->SetMainCamera(m_Player->AddComponent<Blainn::CameraComponent>(800, 600));
	}

	void SolarAppLayer::OnUpdate(const Blainn::GameTimer& gt)
	{
		//auto transform =m_Sun->GetComponent<Blainn::TransformComponent>();
		//auto prevRot = transform->GetWorldQuat();
		//DirectX::SimpleMath::Quaternion deltaQuat =
		//	DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(10.f * gt.DeltaTime(), 0.f, 0.f);
		//transform->SetWorldQuat(prevRot + deltaQuat);
	}

	void SolarAppLayer::OnEvent(Blainn::Event& event)
	{
	}

	void SolarAppLayer::OnDetach()
	{
	}

}
