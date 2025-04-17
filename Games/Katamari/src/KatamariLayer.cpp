#include "KatamariLayer.h"

#include "Core/Application.h"
#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/CharacterComponents/PlayerInputComponent.h"
#include "Components/ActorComponents/PhysicsComponents/SphereCollisionComponent.h"
#include "Components/ActorComponents/PointLightComponent.h"
#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ActorComponents/TransformComponent.h"
#include "Core/GameObject.h"
#include "DX12/DXModel.h"
#include "Scene/Actor.h"

#include "Player.h"

#include "dx12lib/Scene.h"
#include "dx12lib/SceneNode.h"
#include "dx12lib/Material.h"
#include "dx12lib/Mesh.h"

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
	auto floorScene = plane->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/plane/Plane.gltf");
	dx12lib::MaterialProperties matProp = dx12lib::Material::White;
	matProp.SpecularPower = 5.f;
	matProp.Ambient = DirectX::SimpleMath::Vector4(0.1f);
	std::shared_ptr<dx12lib::Material> mat = std::make_shared<dx12lib::Material>(matProp);
	floorScene->GetModel()->GetScene()->GetRootNode()->GetMesh()->SetMaterial(mat);

	plane->GetComponent<TransformComponent>()->SetWorldPosition({ 0.f, -1.0f, 0.f });
	plane->GetComponent<TransformComponent>()->SetWorldScale({ 100.f, 1.f, 100.f });

	auto coolCube = std::make_shared<Blainn::GameObject>();
	m_Scene->QueueGameObject(coolCube);
	coolCube->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/CoolTexturedCube.fbx");
	coolCube->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({ 0.f, 0.f, 5.f });
	auto randomObjCol = coolCube->AddComponent<Blainn::SphereCollisionComponent>(1.f);
	randomObjCol->SetCollisionCallback([](std::shared_ptr<CollisionComponent> other) { OutputDebugStringW(L"Colliding with the object\n"); });
	//ball->GetComponent<TransformComponent>()->SetWorldScale({ 0.01f, 0.01f, 0.01f });
	coolCube->GetComponent<TransformComponent>()->SetWorldYawPitchRoll({ -90.0f, 0.0f, 180.0f });

	auto light = std::make_shared<Blainn::GameObject>();
	m_Scene->QueueGameObject(light);
	light->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({ -10.f, 2.f, 2.f });
	light->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/CoolTexturedCube.fbx");
	light->GetComponent<TransformComponent>()->SetWorldScale({ 0.3f, 0.3f, 0.3f });
	auto& pl = light->AddComponent<Blainn::PointLightComponent>()->GetPointLight();
	pl.ConstantAttenuation = 0.f;
	pl.LinearAttenuation = 0.4f;
	

	auto light2 = std::make_shared<Blainn::GameObject>();
	m_Scene->QueueGameObject(light2);
	light2->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({ 10.f, 5.f, 0.f });
	light2->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/CoolTexturedCube.fbx");
	light2->GetComponent<TransformComponent>()->SetWorldScale({ 0.3f, 0.3f, 0.3f });
	light2->AddComponent<Blainn::PointLightComponent>();

	//auto coolCubeModel = std::make_shared<Blainn::DXModel>("../../Resources/Models/CoolTexturedCube.fbx");
	//auto coolCubeModel = std::make_shared<Blainn::DXModel>("../../Resources/Models/dragonkin/scene.gltf");
	//for (int i = -50; i <= 50; i += 4)
	//{
	//	for (int j = -50; j <= 50; j += 4)
	//	{
	//		auto instancedCube = std::make_shared<Blainn::GameObject>();
	//		m_Scene->QueueGameObject(instancedCube);

	//		instancedCube->AddComponent<Blainn::StaticMeshComponent>(coolCubeModel);
	//		instancedCube->AddComponent<Blainn::TransformComponent>()->SetWorldPosition({ float(i), 0.f, float(j)});
	//		instancedCube->GetComponent<TransformComponent>()->SetWorldScale({ 0.008f, 0.008f, 0.008f });
	//		instancedCube->AddComponent<Blainn::SphereCollisionComponent>(1.f);
	//	}
	//}

	auto guy = std::make_shared<Blainn::Actor>();
	m_Scene->QueueGameObject(guy);
	guy->AddComponent<StaticMeshComponent>("../../Resources/Models/dragonkin/scene.gltf");
	guy->GetComponent<TransformComponent>()->SetWorldPosition({ 10.f, 0.0f, 10.f });
	guy->AddComponent<SphereCollisionComponent>(2.f);
}
