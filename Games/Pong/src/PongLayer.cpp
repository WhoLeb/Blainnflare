#include "PongLayer.h"

#include "Core/Application.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/GameTimer.h"
#include "DX12/DXGraphicsPrimitive.h"
#include "SimpleMath.h"
#include "Scene/Actor.h"
#include "Util/MathHelper.h"


#include <memory>
#include <vector>

using namespace DirectX;

namespace Pong
{
	static std::shared_ptr<Blainn::DXGraphicsPrimitive> CreateCube(float side = 2.f);

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

		auto box = CreateCube(2.f);
		m_BoxActor = std::make_shared<Blainn::Actor>();
		m_BoxActor->SetModel(box);

		m_Scene->AddOpaqueActor(m_BoxActor);
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

		DirectX::SimpleMath::Vector3 prevPos = m_BoxActor->GetTransform().position;
		m_BoxActor->SetWorldPosition(prevPos + DirectX::SimpleMath::Vector3(0.1, 0.f, 0.f) * gt.DeltaTime());
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
		int x = e.GetX();
		int y = e.GetY();

		float dx = XMConvertToRadians(0.25 * static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25 * static_cast<float>(y - m_LastMousePos.y));

		m_Theta += dx;
		m_Phi += dy;

		m_Phi = Blainn::MathHelper::Clamp(m_Phi, 0.1f, Blainn::MathHelper::Pi - 0.1f);

		m_LastMousePos.x = x;
		m_LastMousePos.y = y;

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

	std::shared_ptr<Blainn::DXGraphicsPrimitive> CreateCube(float side)
	{
		std::vector<Blainn::DXGraphicsPrimitive::Vertex> vertices =
		{
		{ SimpleMath::Vector3(-side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 1.f, 1.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 0.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, +side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 0.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, -side / 2, -side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 1.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 0.f, 1.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 1.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, +side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 1.f, 1.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side / 2, -side / 2, +side / 2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 0.f, 1.f),	SimpleMath::Vector2(0, 0)}
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

		return std::make_shared<Blainn::DXGraphicsPrimitive>(vertices, &indices);
	}
}
