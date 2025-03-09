#pragma once

#include "Core/GameObject.h"

namespace solar
{
	class Planet : public Blainn::GameObject
	{
		using Super = Blainn::GameObject;
	public:
		Planet() {}

		void OnUpdate(const Blainn::GameTimer& gt) override;

	private:
		float radius = 10.f;
	};
}
