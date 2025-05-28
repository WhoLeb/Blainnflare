#pragma once
#include "SimpleMath.h"

#define CASCADE_COUNT 4

namespace Blainn
{
	struct alignas(16) PerObjectData
	{
		DirectX::SimpleMath::Matrix WorldMatrix;
	};

	struct alignas(16) CascadeData
	{
		DirectX::SimpleMath::Matrix viewProjMats[CASCADE_COUNT];
		float distances[CASCADE_COUNT];
	};

	struct alignas(16) PerPassData
	{
		DirectX::SimpleMath::Matrix View = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix InvView = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix Proj = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix InvProj = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix ViewProj = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix InvViewProj = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Vector3 EyePosW{ 0.f, 0.f, 0.f };
		float padding = 0.f;
		DirectX::SimpleMath::Vector2 RenderTargetSize{ 0.f, 0.f };
		DirectX::SimpleMath::Vector2 InvRenderTargetSize{ 0.f, 0.f };
		float NearZ = 0.f;
		float FarZ = 0.f;
		float TotalTime = 0.f;
		float DeltaTime = 0.f;
	};
	
	struct LightProperties
	{
		uint32_t NumPointLights;
		uint32_t NumSpotLights;
		uint32_t NumDirectionalLights;
	};
}

