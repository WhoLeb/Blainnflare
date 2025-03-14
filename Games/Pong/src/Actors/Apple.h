#pragma once

#include "DX12/DXGraphicsPrimitive.h"

#include "Scene/Actor.h"

#include <functional>

namespace Pong
{
	class Ball;

	class Apple : public Blainn::Actor
	{
		using Super = Blainn::Actor;
		using CallbackOnIntersect = std::function<void(Apple*)>;
	public:
		Apple(const std::vector<std::shared_ptr<Ball>>& balls, const CallbackOnIntersect& callback);

		void OnRender() override;
		void OnUpdate(const Blainn::GameTimer& gt) override;
	private:
		std::vector<std::shared_ptr<Ball>> m_Balls;

		bool m_bWasIntersecting = false;
		CallbackOnIntersect m_Callback;
	};
}
