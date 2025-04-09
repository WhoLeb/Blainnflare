#include "KatamariLayer.h"

#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/CharacterComponents/PlayerInputComponent.h"
#include "Components/ActorComponents/PhysicsComponents/SphereCollisionComponent.h"
#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Scene/Actor.h"
#include "Core/Application.h"
#include "Core/GameObject.h"

#include "Player.h"

using namespace Blainn;

KatamariLayer::KatamariLayer()
	: m_Scene(Blainn::Application::Get().GetScene())
{
}

void KatamariLayer::OnAttach()
{
	auto player = std::make_shared<Player>();
	//camera->AddComponent<PlayerInputComponent>();
	m_Scene->QueueGameObject(player);
	m_Scene->SetMainCamera(player->GetCameraComponent());
	m_Scene->SetPlayerCollision(player->GetCollisionComponent());

	auto plane = std::make_shared<Blainn::Actor>();
	m_Scene->QueueGameObject(plane);
	plane->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/Plane.fbx");
	plane->GetComponent<TransformComponent>()->SetWorldPosition({ 0.f, -1.0f, 0.f });
	plane->GetComponent<TransformComponent>()->SetWorldScale({ 100.f, 1.f, 100.f });

	auto coolCube = std::make_shared<Blainn::GameObject>();
	m_Scene->QueueGameObject(coolCube);
	//ball->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/dragonkin/scene.gltf");
	coolCube->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/CoolTexturedCube.fbx");
	coolCube->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({ 0.f, 0.f, 5.f });
	auto randomObjCol = coolCube->AddComponent<Blainn::SphereCollisionComponent>(1.f);
	randomObjCol->SetCollisionCallback([](std::shared_ptr<CollisionComponent> other) { OutputDebugStringW(L"Colliding with the object\n"); });
	//ball->GetComponent<TransformComponent>()->SetWorldScale({ 0.01f, 0.01f, 0.01f });
	coolCube->GetComponent<TransformComponent>()->SetWorldYawPitchRoll({ -90.0f, 0.0f, 180.0f });

	//auto coolCubeModel = std::make_shared<Blainn::DXModel>("../../Resources/Models/CoolTexturedCube.fbx");
	auto coolCubeModel = std::make_shared<Blainn::DXModel>("../../Resources/Models/dragonkin/scene.gltf");
	for (int i = -50; i <= 50; i += 4)
	{
		for (int j = -50; j <= 50; j += 4)
		{
			auto instancedCube = std::make_shared<Blainn::GameObject>();
			m_Scene->QueueGameObject(instancedCube);

			instancedCube->AddComponent<Blainn::StaticMeshComponent>(coolCubeModel);
			instancedCube->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({ float(i), 0.f, float(j)});
			instancedCube->GetComponent<TransformComponent>()->SetWorldScale({ 0.008f, 0.008f, 0.008f });
			instancedCube->AddComponent<Blainn::SphereCollisionComponent>(1.f);
		}
	}

	auto guy = std::make_shared<Blainn::Actor>();
	m_Scene->QueueGameObject(guy);
	guy->AddComponent<StaticMeshComponent>("../../Resources/Models/dragonkin/scene.gltf");
	guy->GetComponent<TransformComponent>()->SetWorldScale({ 0.01f, 0.01f, 0.01f });
	guy->GetComponent<TransformComponent>()->SetWorldPosition({ 10.f, 0.0f, 10.f });
	guy->AddComponent<SphereCollisionComponent>(2.f);
}
