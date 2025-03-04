#pragma once

#include "Scene/Actor.h"

namespace Pong
{

	class Ball : public Blainn::Actor
	{
		using Super = Blainn::Actor;
	public:
		Ball();
		void OnUpdate(const Blainn::GameTimer& gt) override;

	private:
		const float m_Speed = 5.f;
	};

}

