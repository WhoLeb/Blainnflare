#include "PongLayer.h"

#include "Core/Application.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/GameTimer.h"
#include "DX12/DXStaticMesh.h"
#include "SimpleMath.h"
#include "Scene/Actor.h"
#include "Util/MathHelper.h"

#include "Actors/Ball.h"
#include "Actors/PlayerRacket.h"

#include <iostream>

#include <memory>
#include <vector>

using namespace DirectX;

namespace Pong
{
	static std::shared_ptr<Blainn::DXStaticMesh> CreateCube(float side = 2.f, const DirectX::SimpleMath::Color& color = {1.f, 1.f, 1.f});

	PongLayer::PongLayer()
		: Blainn::Layer("PongLayer")
		, m_Scene(Blainn::Application::Get().GetScene())
		, m_EyePos(m_Scene->GetEyePos())
		, m_View(m_Scene->GetView())
		, m_Proj(m_Scene->GetProj())
	{}

	void PongLayer::OnAttach()
	{
		Super::OnAttach();

		auto box = CreateCube(2.f, {1.f, 1.f, 1.f});
#pragma region walls
		{
			auto wall = std::make_shared<Blainn::Actor>();
			wall->SetModel(box);
			wall->SetWorldPosition({ 0.f, 0.f, 10.2f });
			wall->SetScale({ .1f, 7.f, .1f });
			m_Scene->AddOpaqueActor(wall);
		}
		{
			auto wall = std::make_shared<Blainn::Actor>();
			wall->SetModel(box);
			wall->SetWorldPosition({ 0.f, 0.f, -10.2f });
			wall->SetScale({ .1f, 7.f, .1f });
			m_Scene->AddOpaqueActor(wall);
		}
		{
			auto wall = std::make_shared<Blainn::Actor>();
			wall->SetModel(box);
			wall->SetWorldPosition({ 0.f, 7.2f, 0.f });
			wall->SetScale({ 10.f, .2f, 0.1f });
			m_Scene->AddOpaqueActor(wall);
		}
		{
			auto wall = std::make_shared<Blainn::Actor>();
			wall->SetModel(box);
			wall->SetWorldPosition({ 0.f, -7.2f, .0f });
			wall->SetScale({ 10.f, 0.2f, 0.1f });
			m_Scene->AddOpaqueActor(wall);
		}
#pragma endregion
		{
			m_LeftRacket = std::make_shared<PlayerRacket>(Blainn::KeyCode::W, Blainn::KeyCode::S);
			m_LeftRacket->SetModel(box);
			m_LeftRacket->SetWorldPosition({ 0.f, 0.f, 10.f });
			m_LeftRacket->SetScale({ .1f, 1.3f, 0.1f });
			m_Scene->AddOpaqueActor(m_LeftRacket);
		}
		{
			m_RightRacket = std::make_shared<PlayerRacket>(Blainn::KeyCode::UpArrow, Blainn::KeyCode::DownArrow);
			m_RightRacket->SetModel(box);
			m_RightRacket->SetWorldPosition({ 0.f, 0.f, -10.f });
			m_RightRacket->SetScale({ .1f, 1.3f, 0.1f });
			m_Scene->AddOpaqueActor(m_RightRacket);
		}
		{
			auto ballMesh = CreateCube(0.4f);
			m_Ball = std::make_shared<Ball>([this](Pong::HitInfo hit) { this->OnWallHit(hit); });
			m_Ball->SetModel(ballMesh);
			m_Ball->SetWorldPosition({ 0.f, 0.f, 0.f });
			m_Scene->AddOpaqueActor(m_Ball);
		}
	}

	void PongLayer::OnDetach()
	{
		Super::OnDetach();
	}

	void PongLayer::OnUpdate(const Blainn::GameTimer& gt)
	{
		Super::OnUpdate(gt);

		float xa = m_Radius * sinf(m_Phi) * cosf(m_Theta);
		float za = m_Radius * sinf(m_Phi) * sinf(m_Theta);
		float ya = m_Radius * cosf(m_Phi);

		// Build the view matrix.
		DirectX::SimpleMath::Vector3 pos{ xa, ya, za };
		DirectX::SimpleMath::Vector3 target{ 0.f };
		DirectX::SimpleMath::Vector3 up{ 0.0f, 1.0f, 0.0f };

		DirectX::SimpleMath::Matrix view = DirectX::SimpleMath::Matrix::CreateLookAt(pos, target, up);
		m_View = view;
		m_EyePos = pos;

		m_Scene->UpdateCamera(m_EyePos, m_View, m_Proj);

		//DirectX::SimpleMath::Vector3 prevPos = m_BoxActor->GetTransform().position;
		//m_BoxActor->SetWorldPosition(prevPos + DirectX::SimpleMath::Vector3(0.1, 0.f, 0.f) * gt.DeltaTime());
	}

	void PongLayer::OnEvent(Blainn::Event& e)
	{
		Super::OnEvent(e);
		Blainn::EventDispatcher dispatcher(e);

		dispatcher.Dispatch<Blainn::WindowResizeEvent>([this](Blainn::WindowResizeEvent& e) {return OnWindowResize(e); });
		dispatcher.Dispatch<Blainn::MouseMovedEvent>([this](Blainn::MouseMovedEvent& e) {return OnMouseMove(e); });
	}

	bool PongLayer::OnMouseMove(Blainn::MouseMovedEvent& e)
	{
		//int x = e.GetX();
		//int y = e.GetY();

		//float dx = XMConvertToRadians(0.25 * static_cast<float>(x - m_LastMousePos.x));
		//float dy = XMConvertToRadians(0.25 * static_cast<float>(y - m_LastMousePos.y));

		//m_Theta += dx;
		//m_Phi += dy;

		//m_Phi = Blainn::MathHelper::Clamp(m_Phi, 0.1f, Blainn::MathHelper::Pi - 0.1f);

		//m_LastMousePos.x = x;
		//m_LastMousePos.y = y;

		return false;
	}

	bool PongLayer::OnWindowResize(Blainn::WindowResizeEvent& e)
	{
		if (e.GetWidth() > 0 && e.GetHeight() > 0)
		{
			float aspectRatio = float(e.GetWidth()) / float(e.GetHeight());
			m_Proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(0.25f * Blainn::MathHelper::Pi, aspectRatio, 0.0001f, 100000.f);
			m_Scene->UpdateCamera(m_EyePos, m_View, m_Proj);
		}
		return false;
	}

	void PongLayer::OnWallHit(Pong::HitInfo hit)
	{
		const auto racketYScale = m_LeftRacket->GetTransform().scale.y;
		switch (hit.wall)
		{
		case WallType::LeftWall:
			if ((hit.hitPos.y > m_LeftRacket->GetTransform().position.y + racketYScale)
				|| (hit.hitPos.y < m_LeftRacket->GetTransform().position.y - racketYScale))
			{
				m_Ball->ResetSpeed();
				m_PlayerScore.RightScore++;
				PrintScore();
			}
			break;
		case WallType::RightWall:
			if ((hit.hitPos.y > m_RightRacket->GetTransform().position.y + racketYScale)
				|| (hit.hitPos.y < m_RightRacket->GetTransform().position.y - racketYScale))
			{
				m_Ball->ResetSpeed();
				m_PlayerScore.LeftScore++;
				PrintScore();
			}
			break;
		}
	}

	void PongLayer::PrintScore()
	{
		std::cout << "Left: " << m_PlayerScore.LeftScore << ", Right: " <<
			m_PlayerScore.RightScore << "\n";
	}

	std::shared_ptr<Blainn::DXStaticMesh> CreateCube(float side, const SimpleMath::Color& color)
	{
		std::vector<Blainn::DXStaticMesh::Vertex> vertices =
		{
		//{ SimpleMath::Vector3(-side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 1.f, 1.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(-side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 0.f, 0.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(+side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 0.f, 0.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(+side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 1.f, 0.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(-side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 0.f, 1.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(-side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 1.f, 0.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(+side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 1.f, 1.f),	SimpleMath::Vector2(0, 0)},
		//{ SimpleMath::Vector3(+side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 0.f, 1.f),	SimpleMath::Vector2(0, 0)}
		{ SimpleMath::Vector3(-side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), color,	SimpleMath::Vector2(0, 0)}
		};

		std::vector<UINT32> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
			1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};

		return std::make_shared<Blainn::DXStaticMesh>(vertices, &indices);
	}
}
