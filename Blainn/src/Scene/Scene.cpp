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

		auto collisions = ComponentManager::Get().GetComponents<CollisionComponent>();

		if (m_PlayerCollision)
		{
			for (auto& collision : collisions)
			{
				if (m_PlayerCollision == collision) continue;

				if (m_PlayerCollision->Intersects(collision))
				{
					m_PlayerCollision->OnCollision(collision);
				}
			}
		}

		// * runs N^2 times, well, very straightforward...
		//for (auto& collisionA : collisions)
		//{
		//	for (auto& collisionB : collisions)
		//	{
		//		if (collisionA == collisionB) continue;

		//		if (collisionA->Intersects(collisionB))
		//		{
		//			if (collisionA && collisionB)
		//				collisionA->OnCollision(collisionB);
		//		}
		//	}
		//}

		// * this runs less times but i am creating the vector every frame which is not ideal. probably switcing
		// * to an octree and reducing calculations this way is way more efficient and may be beneficial
		// * this still runs N^2 times
		//std::vector<std::shared_ptr<CollisionComponent>> colVec(collisions.begin(), collisions.end());
		//for (int a = 0; a < colVec.size(); ++a)
		//{
		//	for (int b = a + 1; b < colVec.size(); ++b)
		//	{
		//		if (colVec[a]->Intersects(colVec[b]))
		//		{
		//			colVec[a]->OnCollision(colVec[b]);
		//			colVec[b]->OnCollision(colVec[a]);
		//		}
		//	}
		//}
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
