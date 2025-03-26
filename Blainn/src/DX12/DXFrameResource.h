#pragma once

#include "DXUploadBuffer.h"

#include "SimpleMath.h"

const UINT MaxLights = 16;

namespace Blainn
{
	struct Light
	{
		DirectX::SimpleMath::Vector3 Strength;
		float falloffStart;
		DirectX::SimpleMath::Vector3 Direction;
		float falloffEnd;
		DirectX::SimpleMath::Vector3 Position;
		float SpotPower;
	};

	struct PassConstants
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

		DirectX::SimpleMath::Color AmbientLight = { 0.f, 0.f, 0.f, 1.f };
		Light Lights[MaxLights];
	};

	struct ObjectConstants
	{
		DirectX::SimpleMath::Matrix World = DirectX::SimpleMath::Matrix::Identity;
	};

	struct MaterialConstants
	{
		DirectX::SimpleMath::Color DiffuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
		DirectX::SimpleMath::Vector3 Frensel = { 0.01f, 0.01f, 0.01f };
		float Roughness = 0.25f;
		DirectX::SimpleMath::Matrix MatTransform = DirectX::SimpleMath::Matrix::Identity;
	};

	class DXFrameResource
	{
	public:
		DXFrameResource(UINT passCount, UINT objectCount);
		DXFrameResource(const DXFrameResource& other) = delete;
		DXFrameResource& operator=(const DXFrameResource& other) = delete;
		~DXFrameResource() = default;

		ID3D12Resource* GetPassBufferResource() const { return m_PassConstantBuffer->GetResource(); }
		ID3D12Resource* GetObjectBufferResource() const { return m_ObjectsConstantBuffer->GetResource(); }
		ID3D12Resource* GetMaterialsBufferResource() const { return m_MaterialsConstantBuffer->GetResource(); }
		DXUploadBuffer<PassConstants>* GetPassConstantBuffer() const { return m_PassConstantBuffer.get(); }
		DXUploadBuffer<ObjectConstants>* GetObjectConstantBuffer() const { return m_ObjectsConstantBuffer.get(); }
		DXUploadBuffer<MaterialConstants>* GetMaterialsConstantBuffer() const { return m_MaterialsConstantBuffer.get(); }

		UINT64 GetFence() const { return Fence; }
		void SetFence(UINT64 value) { Fence = value; }
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAlloc() const { return m_CmdListAllocator; }

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CmdListAllocator;

		std::unique_ptr<DXUploadBuffer<PassConstants>> m_PassConstantBuffer;
		std::unique_ptr<DXUploadBuffer<ObjectConstants>> m_ObjectsConstantBuffer;
		std::unique_ptr<DXUploadBuffer<MaterialConstants>> m_MaterialsConstantBuffer;

		UINT64 Fence = 0;
	};

}
