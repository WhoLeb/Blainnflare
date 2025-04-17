#include "pch.h"
#include "Actor.h"

#include "Components/ActorComponents/TransformComponent.h"

namespace Blainn
{
    void Actor::OnAttach()
    {
        Super::OnAttach();
        transform = AddComponent<TransformComponent>();
    }
}

