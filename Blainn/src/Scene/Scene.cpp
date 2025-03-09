#include "pch.h"
#include "Scene.h"

#include "Components/ActorComponents/CharacterComponents/CameraComponent.h"
#include "Components/ActorComponents/StaticMeshComponent.h"
#include "Core/Application.h"
#include "Core/GameObject.h"
#include "Core/GameTimer.h"

extern const UINT32 g_NumObjects;

namespace Blainn
{
	Scene::Scene()
	{
		for (UINT32 i = g_NumObjects - 1; i != 0; i--)
		{
			m_FreeBufferIndices.push(i);
		}
	}

	void Scene::UpdateScene(const GameTimer& gt)
	{
		ProcessPendingRemovals();
		for (auto& object : m_AllObjects)
			object->OnUpdate(gt);
		ProcessPendingAdditions();
	}

	void Scene::RenderScene()
	{
	}

	void Scene::QueueGameObject(std::shared_ptr<GameObject> gameObject)
	{
		m_PendingAdditions.push_back(gameObject);
	}

	void Scene::RemoveGameObject(std::shared_ptr<GameObject> gameObject)
	{
		m_PendingRemovals.push_back(gameObject);
	}

	UINT32 Scene::GetCBIdx(UUID uuid) const
	{
		auto it = m_UUIDToCBIndex.find(uuid);
		return (it != m_UUIDToCBIndex.end()) ? it->second : UINT32_MAX;
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

		auto meshComponents = obj->GetComponents<StaticMeshComponent>();
		for (auto meshComponent : meshComponents)
		{
			m_AllRenderObjects.push_back(meshComponent);

			// For transparent objects. Not done yet so TODO
			//if (meshComponent->IsTransparent())
			//	m_TransparentObjects.push_back(meshComponent);
			//else
			m_OpaqueObjects.push_back(meshComponent);

			AssignCBIdx(obj->GetUUID());
		}

		obj->OnBegin();
	}

	void Scene::RemoveFromScene(std::shared_ptr<GameObject> obj)
	{
		auto it = std::find(m_AllObjects.begin(), m_AllObjects.end(), obj);
		if (it != m_AllObjects.end())
			m_AllObjects.erase(it);

		auto meshComponents = obj->GetComponents<StaticMeshComponent>();

		for (auto& meshComponent : meshComponents)
		{
			m_AllRenderObjects.erase(std::remove(m_AllRenderObjects.begin(), m_AllRenderObjects.end(), meshComponent), m_AllRenderObjects.end());
			m_OpaqueObjects.erase(std::remove(m_OpaqueObjects.begin(), m_OpaqueObjects.end(), meshComponent), m_OpaqueObjects.end());
			m_TransparentObjects.erase(std::remove(m_TransparentObjects.begin(), m_TransparentObjects.end(), meshComponent), m_TransparentObjects.end());

			ReleaseCBIdx(obj->GetUUID());
		}

		obj->OnDestroy();

		if (auto parent = obj->GetParent())
			parent->RemoveChild(obj);
	}


	UINT32 Scene::AssignCBIdx(UUID uuid)
	{
		if (m_FreeBufferIndices.empty())
			throw std::runtime_error("No free constant buffer indices available");

		UINT32 bufferIndex = m_FreeBufferIndices.top();
		m_FreeBufferIndices.pop();
		m_UUIDToCBIndex[uuid] = bufferIndex;

		return bufferIndex;
	}

	void Scene::ReleaseCBIdx(UUID uuid)
	{
		auto it = m_UUIDToCBIndex.find(uuid);
		if (it != m_UUIDToCBIndex.end())
		{
			UINT32 bufferIndex = it->second;
			m_FreeBufferIndices.push(bufferIndex);
			m_UUIDToCBIndex.erase(it);
		}
	}

}
