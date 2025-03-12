#pragma once

#include "SimpleMath.h"
#include "Core/GameObject.h"

#include "Components/ActorComponents/TransformComponent.h"

#include <memory>

extern const int g_NumFrameResources;

namespace Blainn
{
	class DXStaticMesh;
	class TransformComponent;
	class Actor : public GameObject
	{
		using Super = GameObject;
	public:
		Actor();
		~Actor() noexcept override {}
		
	protected:

		TransformComponent* transform;
	private:

	};
}
