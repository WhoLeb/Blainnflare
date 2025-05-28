#include "pch.h"
#include "SpotLightComponent.h"

using namespace Blainn;

SpotLightComponent::SpotLightComponent(std::shared_ptr<GameObject> owner, SpotLight* sl)
	: Super(owner)
{
	if (sl)
		m_SpotLight = *sl;
	else
		m_SpotLight = SpotLight();
}
