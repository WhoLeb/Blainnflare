#pragma once

#include "Actor.h"

#include <vector>


namespace Blainn
{
	class GameTimer;

	class Scene
	{
	public:
		Scene() {}
		Scene(const Scene& other) = delete;
		Scene& operator=(const Scene& other) = delete;

		void UpdateScene(const GameTimer& gt);
		void RenderScene();

		void UpdateCamera(
			const DirectX::SimpleMath::Vector3& eyePos,
			const DirectX::SimpleMath::Matrix& view,
			const DirectX::SimpleMath::Matrix& proj
		);

		void AddOpaqueActor(std::shared_ptr<Actor> actor);
		void AddTransparentActor(std::shared_ptr<Actor> actor);

		std::vector<std::shared_ptr<Actor>> GetAllActors() const { return m_AllActors; }
		std::vector<std::shared_ptr<Actor>> GetOpaqueActors() const { return m_OpaqueActors; }
		std::vector<std::shared_ptr<Actor>> GetTransparentActors() const { return m_TransparentActors; }

		DirectX::SimpleMath::Vector3 GetEyePos() const { return m_EyePos; }
		DirectX::SimpleMath::Matrix GetView() const { return m_View; }
		DirectX::SimpleMath::Matrix GetProj() const { return m_Proj; }
	protected:
		std::vector<std::shared_ptr<Actor>> m_AllActors;

		std::vector<std::shared_ptr<Actor>> m_OpaqueActors;
		std::vector<std::shared_ptr<Actor>> m_TransparentActors;

		// TODO: replace with a scene camera
		DirectX::SimpleMath::Vector3 m_EyePos{};
		DirectX::SimpleMath::Matrix m_View{};
		DirectX::SimpleMath::Matrix m_Proj{};
	};
}
