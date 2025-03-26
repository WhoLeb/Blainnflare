#pragma once

#include "Core/UUID.h"
#include "Core/MaterialIndexManager.h"

#include "SimpleMath.h"

#include <string>

extern const int g_NumFrameResources;

namespace Blainn
{

	class DXMaterial
	{
	public:
		DXMaterial()
		{
			Name = std::to_string(uuid);
			MaterialIndexManager::Get().AssignCBIdx(uuid);
		}
		~DXMaterial()
		{
			MaterialIndexManager::Get().ReleaseCBIdx(uuid);
		}

		UUID uuid;

		std::string Name;

		int DiffuseSrvHeapIndex = -1;

		int NumFramesDirty = g_NumFrameResources;

		DirectX::SimpleMath::Color DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
		DirectX::SimpleMath::Vector3 Fresel = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;

		DirectX::SimpleMath::Matrix MatTransform = DirectX::SimpleMath::Matrix::Identity;
	};

}
