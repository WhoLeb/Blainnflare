#pragma once

#include "SimpleMath.h"
#include "Core/GameObject.h"

#include <memory>

extern const int g_NumFrameResources;

namespace Blainn
{
	class DXStaticMesh;
	class Actor : public GameObject
	{
		using Super = GameObject;
	public:
		Actor() = default;
		~Actor() noexcept override {}
		
	protected:
		virtual void OnRender();

	private:

	};
}
