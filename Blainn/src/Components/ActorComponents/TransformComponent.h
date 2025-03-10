#pragma once

#include "Components/Component.h"

#include "SimpleMath.h"

extern const int g_NumFrameResources;

namespace Blainn
{

	struct Transform
	{
		DirectX::SimpleMath::Vector3 Position = { 0.f, 0.f, 0.f };
		DirectX::SimpleMath::Vector3 Scale = { 1.f, 1.f, 1.f };
		DirectX::SimpleMath::Quaternion Quaternion{};
	};

	class TransformComponent : public Component
	{
	public:
		TransformComponent() {}

		void SetLocalPosition(const DirectX::SimpleMath::Vector3& newLocalPos);
		void SetLocalYawPitchRoll(const DirectX::SimpleMath::Vector3& newLocalRot);
		void SetLocalQuat(const DirectX::SimpleMath::Quaternion& newLocalRotQuat);
		void SetLocalScale(const DirectX::SimpleMath::Vector3& newLocalScale);

		void SetLocalPositionYawPitchRoll(
			const DirectX::SimpleMath::Vector3& newLocalPos,
			const DirectX::SimpleMath::Vector3& newLocalRot
		);
		void SetLocalPositionQuat(
			const DirectX::SimpleMath::Vector3& newLocalPos,
			const DirectX::SimpleMath::Quaternion& newLocalQuat
		);

		void SetWorldPosition(const DirectX::SimpleMath::Vector3& newWorldPos);
		void SetWorldYawPitchRoll(const DirectX::SimpleMath::Vector3& newWorldRot);
		void SetWorldQuat(const DirectX::SimpleMath::Quaternion& newWorldQuat);
		void SetWorldScale(const DirectX::SimpleMath::Vector3& newWorldScale);

		void SetWorldPositionYawPitchRoll(
			const DirectX::SimpleMath::Vector3& newWorldPos,
			const DirectX::SimpleMath::Vector3& newWorldRot
		);
		void SetWorldPositionQuat(
			const DirectX::SimpleMath::Vector3& newWorldPos,
			const DirectX::SimpleMath::Quaternion& newWorldQuat
		);

		DirectX::SimpleMath::Matrix GetWorldMatrix() const { return m_WorldMatrix; }

		DirectX::SimpleMath::Vector3 GetWorldPosition() const { return m_WorldTransform.Position; }
		DirectX::SimpleMath::Vector3 GetWorldYawPitchRoll() const { return m_WorldTransform.Quaternion.ToEuler(); }
		DirectX::SimpleMath::Quaternion GetWorldQuat() const { return m_WorldTransform.Quaternion; }
		DirectX::SimpleMath::Vector3 GetWorldScale() const { return m_WorldTransform.Scale; }

		DirectX::SimpleMath::Vector3 GetLocalPosition() const { return m_LocalTransform.Position; }
		DirectX::SimpleMath::Vector3 GetLocalYawPitchRoll() const { return m_LocalTransform.Quaternion.ToEuler(); }
		DirectX::SimpleMath::Quaternion GetLocalQuat() const { return m_LocalTransform.Quaternion; }
		DirectX::SimpleMath::Vector3 GetLocalScale() const { return m_LocalTransform.Scale; }

		DirectX::SimpleMath::Vector3 GetWorldForwardVector() const { return m_ForwardVector; }
		DirectX::SimpleMath::Vector3 GetWorldRightVector() const { return m_RightVector; }
		DirectX::SimpleMath::Vector3 GetWorldUpVector() const { return m_UpVector; }
		
		int GetFramesDirty() const { return m_NumFramesDirty; }
		void DecreaseFramesDirty() { if (m_NumFramesDirty > 0) m_NumFramesDirty--; }
	private:
		void UpdateWorldMatrix();
	private:
		Transform m_LocalTransform{};
		Transform m_WorldTransform{};
		DirectX::SimpleMath::Matrix m_WorldMatrix = DirectX::SimpleMath::Matrix::Identity;

		DirectX::SimpleMath::Vector3 m_ForwardVector{};
		DirectX::SimpleMath::Vector3 m_RightVector{};
		DirectX::SimpleMath::Vector3 m_UpVector{};

		int m_NumFramesDirty = g_NumFrameResources;
	};
}
