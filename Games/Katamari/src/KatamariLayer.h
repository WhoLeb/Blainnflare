#pragma once

#include "Core/Layer.h"
#include "Scene/Scene.h"

#include <memory>

class KatamariLayer : public Blainn::Layer
{
public:
	KatamariLayer();

	void OnAttach() override;
	void OnUpdate(const Blainn::GameTimer& gt) override;
private:
	std::shared_ptr<Blainn::Scene> m_Scene;
	std::shared_ptr<Blainn::GameObject> m_DirLight;
};
