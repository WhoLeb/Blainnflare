#include "Ball.h"

namespace Pong
{
	Ball::Ball()
	{
		SetForwardVector({ 0.f, 1.f, 1.f });
		SetUpVector({ -1.f, 0.f, 0.f });
	}

	void Ball::OnUpdate(const Blainn::GameTimer& gt)
	{
		SetWorldPosition(
			GetTransform().position + GetForwardVector() * gt.DeltaTime() * m_Speed);
		DirectX::SimpleMath::Vector3 newVector;
		if (GetTransform().position.z >= 10.f)
		{
			newVector = GetForwardVector() * -1;
			newVector.y *= -1;
			SetForwardVector(newVector);
		}
		if (GetTransform().position.z <= -10.f)
		{ 
			newVector = GetForwardVector() * -1;
			newVector.y *= -1;
			SetForwardVector(newVector);
		}
		if (GetTransform().position.y >= 7.f)
		{
			newVector = GetForwardVector() * -1;
			newVector.z *= -1;
			SetForwardVector(newVector);
		}
		if (GetTransform().position.y <= -7.f)
		{ 
			newVector = GetForwardVector() * -1;
			newVector.z *= -1;
			SetForwardVector(newVector);
		}
	}

}
