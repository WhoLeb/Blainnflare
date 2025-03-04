#include "PlayerRacket.h"

#include "Core/Input.h"

#include <iostream>

namespace Pong
{

	void PlayerRacket::OnUpdate(const Blainn::GameTimer& gt)
	{
		UpdateInput(gt);

	}

	void PlayerRacket::UpdateInput(const Blainn::GameTimer& gt)
	{
		DirectX::SimpleMath::Vector3 deltaPos;
		if (Blainn::Input::IsKeyHeld(m_UpKey))
		{
			deltaPos = { 0, m_RacketSpeed, 0 };
			deltaPos *= gt.DeltaTime();
			auto newPos = GetTransform().position + deltaPos;
			if(newPos.y < 7)
				SetWorldPosition(newPos);
		}
		if (Blainn::Input::IsKeyHeld(m_DownKey))
		{
			deltaPos = { 0, -m_RacketSpeed, 0 };
			deltaPos *= gt.DeltaTime();
			auto newPos = GetTransform().position + deltaPos;
			if(newPos.y > -7)
				SetWorldPosition(newPos);
		}
	}

}
