#pragma once

#include "Components/Component.h"
#include "Scene/Light.h"

namespace Blainn
{
	class PointLightComponent : public Component<PointLightComponent>
	{
		using Super = Component<PointLightComponent>;
	public:
		PointLightComponent();

		PointLight GetPointLight() { return m_PointLight; }
	private:
		PointLight m_PointLight;
	};
}
