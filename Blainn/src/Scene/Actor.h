#pragma once

#include "SimpleMath.h"
#include "Core/GameObject.h"

#include <memory>

extern const int g_NumFrameResources;

namespace Blainn
{
	class DXGraphicsPrimitive;

	struct Transform
	{
		DirectX::SimpleMath::Vector3 position = { 0.f, 0.f, 0.f };
		DirectX::SimpleMath::Vector3 rotation = { 0.f, 0.f, 0.f };
		DirectX::SimpleMath::Vector3 scale = { 1.f, 1.f, 1.f };
	};

	class Actor : public GameObject
	{
		using Super = GameObject;
	public:
		Actor() = default;
		~Actor() noexcept override {}
		
		void SetModel(std::shared_ptr<DXGraphicsPrimitive> model);

		void SetIndex(UINT32 idx) { m_ObjectConstantBufferIndex = idx; }

		void SetWorldPosition(const DirectX::SimpleMath::Vector3& newPos);
		void SetWorldRotation(const DirectX::SimpleMath::Vector3& newRot);
		void SetForwardVector(const DirectX::SimpleMath::Vector3& newForwardVector);
		void SetUpVector(const DirectX::SimpleMath::Vector3& newUpVector);
		void SetScale(const DirectX::SimpleMath::Vector3& newScale);


		void SetTransform(const Transform& newTransform);
		void SetWorldMatrix(const DirectX::SimpleMath::Matrix& newWorldMatrix);

		Transform GetTransform() const { return m_Transform; }
		DirectX::SimpleMath::Vector3 GetForwardVector() const { return m_ForwardVector; }
		DirectX::SimpleMath::Vector3 GetUpVector() const { return m_UpVector; }
		DirectX::SimpleMath::Vector3 GetRightVector() const { return m_RightVector; }
	protected:
		virtual void OnRender();

		std::shared_ptr<DXGraphicsPrimitive> m_GraphicsPrimitive;
		DirectX::SimpleMath::Matrix m_WorldMatrix{};

	private:
		void UpdateWorldMatrix();
		int m_NumFramesDirty = g_NumFrameResources;
		UINT32 m_ObjectConstantBufferIndex = -1;

		Transform m_Transform;
		DirectX::SimpleMath::Vector3 m_ForwardVector{ 1.f, 0.f, 0.f };
		DirectX::SimpleMath::Vector3 m_UpVector{ 0.f, 1.f, 0.f };
		DirectX::SimpleMath::Vector3 m_RightVector;

		friend class DXRenderingContext;
	};
}
