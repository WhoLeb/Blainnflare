#pragma once

#include "Components/Component.h"
#include "Scene/Light.h"

namespace Blainn
{
	class DirectionalLightComponent : public Component<DirectionalLightComponent>
	{
		using Super = Component<DirectionalLightComponent>;
	public:
		DirectionalLightComponent() : Super(), m_DirLight() {}

		DirectionalLight& GetDirectionalLight() { return m_DirLight; }
	private:
		DirectionalLight m_DirLight;
	};
}
