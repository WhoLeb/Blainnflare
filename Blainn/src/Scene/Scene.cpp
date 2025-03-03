#include "pch.h"
#include "Scene.h"

#include "Core/Application.h"
#include "Core/GameTimer.h"

namespace Blainn
{
	void Scene::UpdateScene(const GameTimer& gt)
	{
		Application::Get().GetRenderingContext()->UpdateMainPassConstantBuffers(
			gt, m_View, m_Proj, m_EyePos
		);
		Application::Get().GetRenderingContext()->UpdateObjectsConstantBuffers(m_AllActors);
	}

	void Scene::UpdateCamera(const DirectX::SimpleMath::Vector3& eyePos, const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
	{
		m_EyePos = eyePos;
		m_View = view;
		m_Proj = proj;
	}

	void Scene::AddOpaqueActor(std::shared_ptr<Actor> actor)
	{
		m_AllActors.push_back(actor);
		m_OpaqueActors.push_back(actor);
		actor->SetIndex(m_AllActors.size() - 1);
	}

	void Scene::AddTransparentActor(std::shared_ptr<Actor> actor)
	{
		m_AllActors.push_back(actor);
		m_TransparentActors.push_back(actor);
		actor->SetIndex(m_AllActors.size() - 1);
	}
}
