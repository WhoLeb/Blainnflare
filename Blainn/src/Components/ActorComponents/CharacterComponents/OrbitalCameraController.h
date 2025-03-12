#pragma once

#include "SimpleMath.h"

#include "InputComponent.h"

namespace Blainn
{
	class OrbitalInputComponent : public InputComponent
	{
		using Super = InputComponent;

	public:
		OrbitalInputComponent(float sensitivity = 0.05f, float speed = 5.f, float radius = 10.f, float theta = 0.f, float phi = DirectX::XM_PIDIV2) 
			: m_Sensitivity(sensitivity)
			, m_Radius(radius)
			, m_Theta(theta)
			, m_Phi(phi)
			, m_Speed(speed)
		{}

		void HandleInput(const GameTimer& gt) override;

	private:
		float m_Sensitivity;
		float m_Radius;
		float m_Theta, m_Phi;
		float m_Speed;
	};
}
