#pragma once

#include "Core/Events/MouseEvent.h"
#include "Core/Events/ApplicationEvent.h"
#include "Core/Layer.h"
#include "DX12/DXGraphicsPrimitive.h"
#include "Scene/Scene.h"

#include "Actors/PlayerRacket.h"

#include <memory>
#include <vector>


namespace Pong
{
	struct HitInfo;
	class Ball;

	class PongLayer : public Blainn::Layer
	{
		using Super = Blainn::Layer;
	public:
		PongLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(const Blainn::GameTimer& gt) override;
		void OnEvent(Blainn::Event& e) override;

		bool OnMouseMove(Blainn::MouseMovedEvent& e);
		bool OnWindowResize(Blainn::WindowResizeEvent& e);
	private:
		void OnWallHit(Pong::HitInfo hit);
		void PrintScore();

		std::vector<std::shared_ptr<Blainn::DXGraphicsPrimitive>> m_Primitives{};

		std::shared_ptr<Blainn::Scene> m_Scene;

		// TODO: replace with a scene camera
		DirectX::SimpleMath::Vector3 m_EyePos{};
		DirectX::SimpleMath::Matrix m_View{};
		DirectX::SimpleMath::Matrix m_Proj{};

		float m_Theta = 0;// 1.5 * DirectX::XM_PI;
		float m_Phi = DirectX::XM_PIDIV2;//DirectX::XM_PIDIV4;
		float m_Radius = 20.f;

		POINT m_LastMousePos{0, 0};

		std::shared_ptr<PlayerRacket> m_LeftRacket;
		std::shared_ptr<PlayerRacket> m_RightRacket;
		std::shared_ptr<Pong::Ball> m_Ball;

		struct Score
		{
			int LeftScore = 0;
			int RightScore = 0;
		} m_PlayerScore;
	};

}

