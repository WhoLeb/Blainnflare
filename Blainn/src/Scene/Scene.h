#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <stack>
#include <windows.h>

#include "Core/UUID.h"

namespace Blainn
{
	class CameraComponent;
	class CollisionComponent;
	class GameObject;
	class GameTimer;
	class StaticMeshComponent;

	class Scene
	{
	public:
		Scene();
		Scene(const Scene& other) = delete;
		Scene& operator=(const Scene& other) = delete;

		void UpdateScene(const GameTimer& gt);
		void RenderScene();

		template<typename T, typename... Args>
		std::shared_ptr<T> QueueGameObject(Args&&... args)
		{
			static_assert(std::is_base_of<GameObject, T>::value, "T must be a GameObject");

			auto obj = std::make_shared<T>(std::forward<Args>(args)...);
			m_PendingAdditions.push_back(obj);
			obj->OnAttach();
			return obj;
		}
		std::shared_ptr<GameObject> QueueGameObject(std::shared_ptr<GameObject> gameObject);
		void RemoveGameObject(std::shared_ptr<GameObject> gameObject);

		const std::vector<StaticMeshComponent*>& GetRenderObjects() const { return m_AllRenderObjects; }

		void SetMainCamera(std::shared_ptr<CameraComponent> camera) { m_MainCamera = camera; }
		std::shared_ptr<CameraComponent> GetMainCamera() const { return m_MainCamera; }

		void SetPlayerCollision(std::shared_ptr<CollisionComponent> collision) { m_PlayerCollision = collision; }
		std::shared_ptr<CollisionComponent> GetPlayerCollision() const { return m_PlayerCollision; }

	private:
		void ProcessPendingAdditions();
		void ProcessPendingRemovals();

		void RegisterObject(std::shared_ptr<GameObject> obj);
		void RemoveFromScene(std::shared_ptr<GameObject> obj);

	private:
		std::vector<std::shared_ptr<GameObject>> m_AllObjects;
		std::vector<StaticMeshComponent*> m_AllRenderObjects;
		std::vector<StaticMeshComponent*> m_OpaqueObjects;
		std::vector<StaticMeshComponent*> m_TransparentObjects;

		std::vector<std::shared_ptr<GameObject>> m_PendingAdditions;
		std::vector<std::shared_ptr<GameObject>> m_PendingRemovals;

		std::shared_ptr<CameraComponent> m_MainCamera = nullptr;
		std::shared_ptr<CollisionComponent> m_PlayerCollision = nullptr;
	};
}
