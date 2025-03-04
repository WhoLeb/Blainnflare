#pragma once

#include "Scene/Actor.h"
#include "Core/KeyCodes.h"

namespace Pong
{
	class PlayerRacket : public Blainn::Actor
	{
		using Super = Blainn::Actor;
	public:
		PlayerRacket(Blainn::KeyCode upKey, Blainn::KeyCode downKey)
			: Super(), m_UpKey(upKey), m_DownKey(downKey)
		{}

		~PlayerRacket() noexcept override {}

		void OnInit() override {}

		void OnUpdate(const Blainn::GameTimer& gt) override;
		void UpdateInput(const Blainn::GameTimer& gt);
		//void;
	protected:
		Blainn::KeyCode m_UpKey, m_DownKey;
		const float m_RacketSpeed = 4;
		//std::shared_ptr<Blainn::DXGraphicsPrimitive> m_GraphicsPrimitive;
	};
}
