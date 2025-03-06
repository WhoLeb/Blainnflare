#include "Ball.h"

namespace Pong
{
	Ball::Ball(const CallbackFn& callback)
	{
		m_Callback = callback;
		SetForwardVector({ 0.f, 1.f, 1.f });
		SetUpVector({ -1.f, 0.f, 0.f });
	}

	void Ball::OnUpdate(const Blainn::GameTimer& gt)
	{
		m_Timer -= gt.DeltaTime();
		if (m_Timer <= 0)
		{
			m_Speed += m_Acceleration;
			m_Timer = m_InitialTimer;
		}
		auto newPos = GetTransform().position + GetForwardVector() * gt.DeltaTime() * m_Speed;
		DirectX::SimpleMath::Vector3 newVector;
		if (newPos.z >= 10.f)
		{
			newVector = GetForwardVector() * -1;
			newVector.y *= -1;
			SetForwardVector(newVector);

			m_Callback({ GetTransform().position, WallType::LeftWall });
		}
		if (newPos.z <= -10.f)
		{ 
			newVector = GetForwardVector() * -1;
			newVector.y *= -1;
			SetForwardVector(newVector);

			m_Callback({ GetTransform().position, WallType::RightWall });
		}
		if (newPos.y >= 7.f)
		{
			newVector = GetForwardVector() * -1;
			newVector.z *= -1;
			SetForwardVector(newVector);
		}
		if (newPos.y <= -7.f)
		{ 
			newVector = GetForwardVector() * -1;
			newVector.z *= -1;
			SetForwardVector(newVector);
		}
		newPos = GetTransform().position + GetForwardVector() * gt.DeltaTime() * m_Speed;
		SetWorldPosition(newPos);
	}

}
