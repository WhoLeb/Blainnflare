#pragma once

#include "Core/Layer.h"
#include "DX12/DXGraphicsPrimitive.h"

#include <memory>
#include <vector>


namespace Pong
{
	class PongLayer : public Blainn::Layer
	{
	public:
		PongLayer()
			: Blainn::Layer("PongLayer")
		{}

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Blainn::Event& e) override;

		bool OnRender();
	private:
		std::vector<std::shared_ptr<Blainn::DXGraphicsPrimitive>> m_Primitives;
	};
}

