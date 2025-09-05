#include "pch.h"
#include "PointLightComponent.h"

Blainn::PointLightComponent::PointLightComponent(std::shared_ptr<GameObject> owner, PointLight* pl)
	: Super(owner)
{
	if (pl)
		m_PointLight = *pl;
	else
		m_PointLight = PointLight();
}
