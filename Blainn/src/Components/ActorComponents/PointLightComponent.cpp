#include "pch.h"
#include "PointLightComponent.h"

Blainn::PointLightComponent::PointLightComponent(PointLight* pl)
	: Super()
{
	if (pl)
		m_PointLight = *pl;
	else
		m_PointLight = PointLight();
}
