#pragma once

#include "Scene/Actor.h"

#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/PhysicsComponents/SphereCollisionComponent.h"
#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/KatamariInputComponent.h"
#include "Components/KatamariCubeInput.h"

namespace Blainn
{
	class CameraComponent;
}

class Player : public Blainn::Actor
{
	using Super = Blainn::Actor;
public:
	Player() = default;

	void OnAttach() override
	{
		//Super::OnAttach();

		m_Camera = AddChild<Blainn::Actor>();
		m_CameraComponent = m_Camera->AddComponent<Blainn::CameraComponent>(1600, 800);
		auto cameraInput = m_Camera->AddComponent<KatamariCameraInput>();

		m_StaticMesh = AddChild<Blainn::Actor>();
		m_StaticMeshComponent = m_StaticMesh->AddComponent<Blainn::StaticMeshComponent>("../../Resources/Models/CoolTexturedCube.fbx");
		m_StaticMesh->AddComponent<KatamariCubeInput>();
		m_CollisionComponent = m_StaticMesh->AddComponent<Blainn::SphereCollisionComponent>(.5f);

		m_CollisionComponent->SetCollisionCallback([this, cameraInput](std::shared_ptr<Blainn::CollisionComponent> other)
			{
					//OutputDebugStringW(L"Colliding with the player\n");
				auto otherOwner = other->GetOwner();
				auto thisOwner = this->m_StaticMesh;
				if (!otherOwner || !thisOwner) return;
				if (std::find(
					thisOwner->GetChildren().begin(),
					thisOwner->GetChildren().end(),
					otherOwner)
					!= thisOwner->GetChildren().end()
				) return;

				auto otherTransform = otherOwner->GetComponent<Blainn::TransformComponent>();
				auto thisTransform = thisOwner->GetComponent<Blainn::TransformComponent>();
				if (!otherTransform || !thisTransform) return;

				auto relativePos = otherTransform->GetWorldPosition() - thisTransform->GetWorldPosition();

				auto thisWorldRot = thisTransform->GetWorldQuat();
				auto otherWorldRot = otherTransform->GetWorldQuat();
				thisWorldRot.Inverse(thisWorldRot);
				auto localOffset = DirectX::SimpleMath::Vector3::Transform(relativePos, thisWorldRot);
				auto localRot = thisWorldRot * otherWorldRot;

				otherOwner->RemoveComponent(other);
				otherOwner->AttachTo(thisOwner->shared_from_this());
				otherTransform->SetLocalPosition(localOffset);
				otherTransform->SetLocalQuat(localRot);

				auto sphereCollision = dynamic_cast<Blainn::SphereCollisionComponent*>(this->m_CollisionComponent.get());
				if (sphereCollision)
				{
					float oldRadius = sphereCollision->GetRadius();
					float newRadius = std::clamp(oldRadius + 0.05f, 0.f, 6.f);
					sphereCollision->UpdateRadius(newRadius);
				}
				
				float oldCameraRadius = cameraInput->GetRadius();
				float newCameraRadius = std::clamp(oldCameraRadius + 0.1f, 0.f, 25.f);
				cameraInput->UpdateRadius(newCameraRadius);
			});

		Super::OnAttach();
	}

	std::shared_ptr<Blainn::CameraComponent> GetCameraComponent() const { return m_CameraComponent; }
	std::shared_ptr<Blainn::CollisionComponent> GetCollisionComponent() const { return m_CollisionComponent; }

private:
	//KatamariInput* m_InputComponent = nullptr;
	std::shared_ptr<Blainn::Actor> m_Camera;
	std::shared_ptr<Blainn::Actor> m_StaticMesh;
	std::shared_ptr<Blainn::CameraComponent> m_CameraComponent = nullptr;
	std::shared_ptr<Blainn::CollisionComponent> m_CollisionComponent = nullptr;
	std::shared_ptr<Blainn::StaticMeshComponent> m_StaticMeshComponent = nullptr;
};
