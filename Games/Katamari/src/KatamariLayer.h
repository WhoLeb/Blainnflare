#pragma once

#include "Core/Layer.h"
#include "Scene/Scene.h"

#include <memory>

class KatamariLayer : public Blainn::Layer
{
public:
	KatamariLayer();

	void OnAttach() override;
private:
	std::shared_ptr<Blainn::Scene> m_Scene;
};
