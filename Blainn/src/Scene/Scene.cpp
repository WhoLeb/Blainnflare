#include "pch.h"
#include "Scene.h"

#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/PhysicsComponents/CollisionComponent.h"
#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Components/ComponentManager.h"
#include "Core/Application.h"
#include "Core/CBIndexManager.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"

#include <iostream>

extern const UINT32 g_NumObjects;

namespace Blainn
{
	Scene::Scene()
	{
	}

	void Scene::UpdateScene(const GameTimer& gt)
	{
		ProcessPendingRemovals();
		ProcessPendingAdditions();
		for (auto& object : m_AllObjects)
			object->OnUpdate(gt);
		ProcessPendingRemovals();
		ProcessPendingAdditions();

		auto transforms = ComponentManager::Get().GetComponents<TransformComponent>();
		for (auto& transform : transforms)
			transform->OnUpdate(gt);

		auto collisions = ComponentManager::Get().GetComponents<CollisionComponent>();
		for (auto& collisionA : collisions)
		{
			for (auto& collisionB : collisions)
			{
				if (collisionA == collisionB) continue;

				if (collisionA->Intersects(collisionB))
				{
					if(collisionA && collisionB)
						collisionA->OnCollision(collisionB);
				}
			}
		}
	}

	void Scene::RenderScene()
	{
	}

	std::shared_ptr<GameObject> Scene::QueueGameObject(std::shared_ptr<GameObject> gameObject)
	{
		m_PendingAdditions.push_back(gameObject);
		gameObject->OnAttach();
		return gameObject;
	}

	void Scene::RemoveGameObject(std::shared_ptr<GameObject> gameObject)
	{
		m_PendingRemovals.push_back(gameObject);
	}


	void Scene::ProcessPendingAdditions()
	{
		while (!m_PendingAdditions.empty())
		{
			auto obj = m_PendingAdditions.back();
			m_PendingAdditions.pop_back();

			RegisterObject(obj);

			for (auto& child : obj->GetChildren())
				m_PendingAdditions.push_back(child);
		}
	}

	void Scene::ProcessPendingRemovals()
	{
		while (!m_PendingRemovals.empty())
		{
			auto obj = m_PendingRemovals.back();
			m_PendingRemovals.pop_back();

			RemoveFromScene(obj);

			for (auto& child : obj->GetChildren())
				m_PendingRemovals.push_back(child);
		}
	}

	void Scene::RegisterObject(std::shared_ptr<GameObject> obj)
	{
		m_AllObjects.push_back(obj);

		obj->OnBegin();
	}

	void Scene::RemoveFromScene(std::shared_ptr<GameObject> obj)
	{
		auto it = std::find(m_AllObjects.begin(), m_AllObjects.end(), obj);
		if (it != m_AllObjects.end())
			m_AllObjects.erase(it);

		obj->OnDestroy();

		if (auto parent = obj->GetParent())
			parent->RemoveChild(obj);
	}




}
