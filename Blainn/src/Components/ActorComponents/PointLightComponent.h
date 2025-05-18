#pragma once

#include "Components/Component.h"
#include "Scene/Light.h"

namespace Blainn
{
	class PointLightComponent : public Component<PointLightComponent>
	{
		using Super = Component<PointLightComponent>;
	public:
		PointLightComponent(std::shared_ptr<GameObject> owner, PointLight* pl = nullptr);

		void MarkDirty() { m_bIsDirty = true; }
		bool IsDirty() const { return m_bIsDirty; }

		PointLight& GetPointLight() { return m_PointLight; }
	private:
		PointLight m_PointLight;

		bool m_bIsDirty = true;
	};
}
