#include "pch.h"
#include "Actor.h"

#include "DX12/DXStaticMesh.h"

#include "Components/ActorComponents/TransformComponent.h"

namespace Blainn
{
    Actor::Actor()
    {
        transform = AddComponent<TransformComponent>();
    }
}

