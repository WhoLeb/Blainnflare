#include "Apple.h"

#include "Ball.h"
#include "DX12/DXGraphicsPrimitive.h"

#include <iostream>

namespace Pong
{
	Apple::Apple(const std::vector<std::shared_ptr<Ball>>& balls, const CallbackOnIntersect& callback)
	{
		m_Balls = balls;

		m_Callback = callback;
		//m_GraphicsPrimitive = 
	}

	void Apple::OnRender()
	{
		Super::OnRender();
	}

	void Apple::OnUpdate(const Blainn::GameTimer& gt)
	{
		Super::OnUpdate(gt);

		for (auto ball : m_Balls)
		{
			auto scaleYover2 = GetTransform().scale.y / 2;
			auto scaleZover2 = GetTransform().scale.z / 2;
			auto pos = GetTransform().position;
			auto ballPos = ball->GetTransform().position;


			if (!m_bWasIntersecting && (ballPos.y < pos.y + scaleYover2
				&& ballPos.y > pos.y - scaleYover2)
				&& (ballPos.z < pos.z + scaleZover2
					&& ballPos.z > pos.z - scaleZover2))
			{
				m_Callback(this);
				m_bWasIntersecting = true;
			}
			else
				if ((ballPos.y > pos.y + scaleYover2
					|| ballPos.y < pos.y - scaleYover2)
					|| (ballPos.z > pos.z + scaleZover2
						|| ballPos.z < pos.z - scaleZover2))
					m_bWasIntersecting = false;

		}

	}

}
