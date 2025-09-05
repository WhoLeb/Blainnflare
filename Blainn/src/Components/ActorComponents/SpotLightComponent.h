#pragma once

#include <Components/Component.h>
#include <Scene/Light.h>

namespace Blainn
{
    class SpotLightComponent : public Component<SpotLightComponent>
    {
        using Super = Component<SpotLightComponent>;
    public:
        SpotLightComponent(std::shared_ptr<GameObject> owner, SpotLight* sl = nullptr);

    private:
        SpotLight m_SpotLight;
    };
}
