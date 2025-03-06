#pragma once

#include "Scene/Actor.h"

#include <functional>

namespace Pong
{
	enum class WallType
	{
		LeftWall,
		RightWall
	};

	struct HitInfo
	{
		DirectX::SimpleMath::Vector3 hitPos;
		WallType wall;
	};

	class Ball : public Blainn::Actor
	{
		using CallbackFn = std::function<void(HitInfo)>;
		using Super = Blainn::Actor;
	public:
		Ball(const CallbackFn& callback);
		void OnUpdate(const Blainn::GameTimer& gt) override;

		void ResetSpeed() { m_Speed = m_InitialSpeed; }

	private:
		const float m_InitialSpeed = 5.f;
		float m_Speed = 5.f;
		const float m_Acceleration = .7f;
		const float m_InitialTimer = 3.f;
		float m_Timer = 3.f;
		CallbackFn m_Callback;
	};

}

