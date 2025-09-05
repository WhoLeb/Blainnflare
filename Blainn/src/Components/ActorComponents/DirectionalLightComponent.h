#pragma once

#include "Components/Component.h"
#include "Scene/Light.h"

namespace Blainn
{
	class DirectionalLightComponent : public Component<DirectionalLightComponent>
	{
		using Super = Component<DirectionalLightComponent>;
	public:
		DirectionalLightComponent(std::shared_ptr<GameObject> owner, DirectionalLight* dirLight = nullptr)
			: Super(owner)
		{
			if (dirLight)
				m_DirLight = *dirLight;
			else
				m_DirLight = DirectionalLight();
		}

		DirectionalLight& GetDirectionalLight() { return m_DirLight; }
	private:
		DirectionalLight m_DirLight;
	};
}
