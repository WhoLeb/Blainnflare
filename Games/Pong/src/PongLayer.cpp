#include "PongLayer.h"

#include "Core/Application.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "DX12/DXGraphicsPrimitive.h"
#include "SimpleMath.h"

#include <memory>
#include <vector>

using namespace DirectX;

namespace Pong
{
	static std::shared_ptr<Blainn::DXGraphicsPrimitive> CreateCube(float side = 2.f);

	void PongLayer::OnAttach()
	{
		auto box = CreateCube(2.f);
		m_Primitives.push_back(box);
		auto box2 = CreateCube(3.f);
		m_Primitives.push_back(box2);

	}

	void PongLayer::OnDetach()
	{
	}

	void PongLayer::OnUpdate()
	{
	}

	void PongLayer::OnEvent(Blainn::Event& e)
	{
		Blainn::EventDispatcher dispatcher(e);

		dispatcher.Dispatch<Blainn::AppRenderEvent>([this](Blainn::AppRenderEvent& e) { return OnRender(); });
	}

	bool PongLayer::OnRender()
	{
		for (auto primitive : m_Primitives)
			primitive->Draw();
		return false;
	}

	std::shared_ptr<Blainn::DXGraphicsPrimitive> CreateCube(float side)
	{
		static std::vector<Blainn::DXGraphicsPrimitive::Vertex> vertices =
		{
		{ SimpleMath::Vector3(-side/2, -side/2, -side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 1.f, 1.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side/2, +side/2, -side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 0.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side/2, +side/2, -side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 0.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side/2, -side/2, -side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 1.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side/2, -side/2, +side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 0.f, 1.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(-side/2, +side/2, +side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 1.f, 0.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side/2, +side/2, +side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(0.f, 1.f, 1.f),	SimpleMath::Vector2(0, 0)},
		{ SimpleMath::Vector3(+side/2, -side/2, +side/2), SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1.f, 0.f, 1.f),	SimpleMath::Vector2(0, 0)}
		};

		static std::vector<UINT32> indices =
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
		
		return std::make_shared<Blainn::DXGraphicsPrimitive>(Blainn::Application::Get().GetResourceManager(), vertices, &indices);
	}
}
