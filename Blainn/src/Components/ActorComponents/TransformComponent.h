#pragma once

#include "Components/Component.h"

#include "SimpleMath.h"

namespace Blainn
{

	struct Transform
	{
		DirectX::SimpleMath::Vector3 Position = { 0.f, 0.f, 0.f };
		DirectX::SimpleMath::Vector3 Rotation = { 0.f, 0.f, 0.f };
		DirectX::SimpleMath::Vector3 Scale = { 1.f, 1.f, 1.f };
	};

	class TransformComponent : public Component
	{
	public:
		void SetPosition(const DirectX::SimpleMath::Vector3& newPos);
		void SetRotation(const DirectX::SimpleMath::Vector3& newPos);
		void SetScale(const DirectX::SimpleMath::Vector3& newPos);

		DirectX::SimpleMath::Matrix GetWorldMatrix() const { return m_WorldMatrix; }
		DirectX::SimpleMath::Vector3 GetWorldPosition() const { return m_Transform.Position; }
		DirectX::SimpleMath::Vector3 GetWorldRotation() const { return m_Transform.Rotation; }
		DirectX::SimpleMath::Vector3 GetScale() const { return m_Transform.Scale; }
		
		int GetFramesDirty() const { return m_NumFramesDirty; }
		void DecreaseFramesDirty() { if (m_NumFramesDirty > 0) m_NumFramesDirty--; }
	private:
		void UpdateWorldMatrix();
	private:
		Transform m_Transform;
		DirectX::SimpleMath::Matrix m_WorldMatrix = DirectX::SimpleMath::Matrix::Identity;

		int m_NumFramesDirty;
	};
}
