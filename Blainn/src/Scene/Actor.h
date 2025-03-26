#pragma once

#include "SimpleMath.h"
#include "Core/GameObject.h"

#include <memory>

extern const int g_NumFrameResources;

namespace Blainn
{
	class DXStaticMesh;
	class TransformComponent;
	class Scene;

	class Actor : public GameObject
	{
		using Super = GameObject;
	public:
		Actor() {}
		~Actor() noexcept override {}
		
		void OnAttach() override;
	protected:

		std::shared_ptr<TransformComponent> transform = nullptr;
	private:

	};
}
