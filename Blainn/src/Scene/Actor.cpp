#include "pch.h"
#include "Actor.h"

#include "DX12/DXStaticMesh.h"

#include "Components/ActorComponents/TransformComponent.h"

namespace Blainn
{
    void Actor::OnAttach()
    {
        Super::OnAttach();
        transform = AddComponent<TransformComponent>();
    }
}

