#include "pch.h"
#include "TransformComponent.h"

#include "Core/GameObject.h"

#include <iostream>

extern const int g_NumFrameResources;

namespace Blainn
{
	void TransformComponent::OnAttach()
	{
		MarkDirty();
	}

	void TransformComponent::SetLocalPosition(const DirectX::SimpleMath::Vector3& newLocalPos)
	{
		MarkDirty();

		m_LocalTransform.Position = newLocalPos;
	}

	void TransformComponent::SetLocalYawPitchRoll(const DirectX::SimpleMath::Vector3& newLocalRot)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		MarkDirty();

		float yawRad = XMConvertToRadians(newLocalRot.x);
		if (yawRad > XM_2PI)
			yawRad -= XM_2PI;
		else if (yawRad < 0)
			yawRad += XM_2PI;

		float pitchRad = XMConvertToRadians(newLocalRot.y);
		if (pitchRad > XM_2PI)
			pitchRad -= XM_2PI;
		else if (pitchRad < 0)
			pitchRad += XM_2PI;

		float rollRad = XMConvertToRadians(newLocalRot.z);
		if (rollRad > XM_2PI)
			rollRad -= XM_2PI;
		else if (rollRad < 0)
			rollRad += XM_2PI;

		Quaternion localRot = Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
		localRot.Normalize();
		m_LocalTransform.Quaternion = localRot;
	}

	void TransformComponent::SetLocalQuat(const DirectX::SimpleMath::Quaternion& newLocalRotQuat)
	{
		MarkDirty();

		m_LocalTransform.Quaternion = newLocalRotQuat;
		m_LocalTransform.Quaternion.Normalize();
	}

	void TransformComponent::SetLocalScale(const DirectX::SimpleMath::Vector3& newLocalScale)
	{
		MarkDirty();

		m_LocalTransform.Scale = newLocalScale;
	}

	void TransformComponent::SetLocalPositionYawPitchRoll(const DirectX::SimpleMath::Vector3& newLocalPos, const DirectX::SimpleMath::Vector3& newLocalRot)
	{
		MarkDirty();

		using namespace DirectX;

		float yawRad = XMConvertToRadians(newLocalRot.x);
		if (yawRad > XM_2PI)
			yawRad -= XM_2PI;
		else if (yawRad < 0)
			yawRad += XM_2PI;

		float pitchRad = XMConvertToRadians(newLocalRot.y);
		if (pitchRad > XM_2PI)
			pitchRad -= XM_2PI;
		else if (pitchRad < 0)
			pitchRad += XM_2PI;

		float rollRad = XMConvertToRadians(newLocalRot.z);
		if (rollRad > XM_2PI)
			rollRad -= XM_2PI;
		else if (rollRad < 0)
			rollRad += XM_2PI;

		m_LocalTransform.Position = newLocalPos;
		m_LocalTransform.Quaternion = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
	}

	void TransformComponent::SetLocalPositionQuat(const DirectX::SimpleMath::Vector3& newLocalPos, const DirectX::SimpleMath::Quaternion& newLocalQuat)
	{
		MarkDirty();

		m_LocalTransform.Position = newLocalPos;
		m_LocalTransform.Quaternion = newLocalQuat;
		m_LocalTransform.Quaternion.Normalize();
	}

	void TransformComponent::SetWorldPosition(const DirectX::SimpleMath::Vector3& newWorldPos)
	{
		using namespace DirectX::SimpleMath;

		MarkDirty();

		auto parentObj = GetOwner() ? GetOwner()->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Position = newWorldPos;
			return;
		}

		// Else, invert parent's world matrix:
		auto* parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Position = newWorldPos;
			return;
		}
		Matrix invParent = parentTransform->GetWorldMatrix().Invert();

		// Convert world => local
		Vector3 localPos = Vector3::Transform(newWorldPos, invParent);
		m_LocalTransform.Position = localPos;
	}

	void TransformComponent::SetWorldYawPitchRoll(const DirectX::SimpleMath::Vector3& newWorldRot)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		MarkDirty();

		float yawRad = XMConvertToRadians(newWorldRot.x);
		if (yawRad > XM_2PI)
			yawRad -= XM_2PI;
		else if (yawRad < 0)
			yawRad += XM_2PI;

		float pitchRad = XMConvertToRadians(newWorldRot.y);
		if (pitchRad > XM_2PI)
			pitchRad -= XM_2PI;
		else if (pitchRad < 0)
			pitchRad += XM_2PI;

		float rollRad = XMConvertToRadians(newWorldRot.z);
		if (rollRad > XM_2PI)
			rollRad -= XM_2PI;
		else if (rollRad < 0)
			rollRad += XM_2PI;

		Quaternion worldQ = Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
		worldQ.Normalize();

		TransformComponent* parentTransform = GetOwner() ? GetOwner()->GetComponent<TransformComponent>() : nullptr;
		if (parentTransform)
		{
			Quaternion parentWorldRot = parentTransform->GetWorldQuat();

			parentWorldRot.Inverse(parentWorldRot);
			Quaternion localQ = parentWorldRot * worldQ;
			localQ.Normalize();

			m_LocalTransform.Quaternion = localQ;
		}
		else
			m_LocalTransform.Quaternion = worldQ;
	}

	void TransformComponent::SetWorldQuat(const DirectX::SimpleMath::Quaternion& newWorldQuat)
	{
		using namespace DirectX::SimpleMath;
		MarkDirty();

		// If no parent, local=world:
		auto parentObj = GetOwner() ? GetOwner()->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Quaternion = newWorldQuat;
			m_LocalTransform.Quaternion.Normalize();
			return;
		}

		auto* parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Quaternion = newWorldQuat;
			m_LocalTransform.Quaternion.Normalize();
			return;
		}

		Quaternion parentWorldRot = parentTransform->GetWorldQuat();
		parentWorldRot.Normalize();

		// local = conj(parentRot) * worldRot
		//parentWorldRot.Conjugate();
		Quaternion local = newWorldQuat * parentWorldRot;
		local.Normalize();

		m_LocalTransform.Quaternion = local;
	}

	void TransformComponent::SetWorldScale(const DirectX::SimpleMath::Vector3& newWorldScale)
	{
		using namespace DirectX::SimpleMath;

		MarkDirty();

		auto parentObj = GetOwner() ? GetOwner()->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Scale = newWorldScale;
			return;
		}

		auto* parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Scale = newWorldScale;
			return;
		}

		Vector3 parentWorldScale = parentTransform->GetWorldScale();
		m_LocalTransform.Scale = newWorldScale / parentWorldScale;
	}

	void TransformComponent::SetWorldPositionYawPitchRoll(const DirectX::SimpleMath::Vector3& newWorldPos, const DirectX::SimpleMath::Vector3& newWorldRot)
	{
	}

	void TransformComponent::SetWorldPositionQuat(const DirectX::SimpleMath::Vector3& newWorldPos, const DirectX::SimpleMath::Quaternion& newWorldQuat)
	{
	}

	DirectX::SimpleMath::Vector3 TransformComponent::GetWorldYawPitchRoll() const
	{
		using namespace DirectX;
		auto q = m_WorldTransform.Quaternion;

		float r02 = 2.f * q.x * q.z + 2.f * q.w * q.y;
		float r22 = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
		float r12 = 2.f * q.y * q.z - 2.f * q.w * q.x;
		float r10 = 2.f * q.x * q.y + 2.f * q.w * q.z;
		float r11 = q.w * q.w - q.x * q.x + q.y * q.y + q.z * q.z;

		float yaw = XMConvertToDegrees(atan2(r02, r22));
		float pitch = XMConvertToDegrees(asin(-r12));
		float roll = XMConvertToDegrees(atan2(r10, r11));
		return DirectX::SimpleMath::Vector3(yaw, pitch, roll);
	}

	DirectX::SimpleMath::Vector3 TransformComponent::GetLocalYawPitchRoll() const
	{
		using namespace DirectX;
		auto q = m_LocalTransform.Quaternion;

		float r02 = 2.f * q.x * q.z + 2.f * q.w * q.y;
		float r22 = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
		float r12 = 2.f * q.y * q.z - 2.f * q.w * q.x;
		float r10 = 2.f * q.x * q.y + 2.f * q.w * q.z;
		float r11 = q.w * q.w - q.x * q.x + q.y * q.y + q.z * q.z;

		float yaw = XMConvertToDegrees(atan2(r02, r22));
		float pitch = XMConvertToDegrees(asin(-r12));
		float roll = XMConvertToDegrees(atan2(r10, r11));
		return DirectX::SimpleMath::Vector3(yaw, pitch, roll);
	}

	void TransformComponent::MarkDirty()
	{
		if (!m_bIsTransformDirty)
		{
			m_bIsTransformDirty = true;


			if (auto* owner = GetOwner())
			{
				for (auto& child : owner->GetChildren())
				{
					auto* childTransform = child->GetComponent<TransformComponent>();
					if (childTransform)
						childTransform->MarkDirty();
				}
			}
		}
	}

	void TransformComponent::UpdateWorldMatrix()
	{
		if (!m_bIsTransformDirty)
			return;

		using namespace DirectX;
		using namespace SimpleMath;
		m_NumFramesDirty = g_NumFrameResources;

		Matrix scale = Matrix::CreateScale(m_LocalTransform.Scale);
		Matrix rot = Matrix::CreateFromQuaternion(m_LocalTransform.Quaternion);
		Matrix trans = Matrix::CreateTranslation(m_LocalTransform.Position);


		Matrix localMat = (scale * rot * trans);

		if (auto* owner = GetOwner())
		{
			auto parentObj = owner->GetParent();
			if (parentObj)
			{
				TransformComponent* parentTransform = parentObj->GetComponent<TransformComponent>();
				if (parentTransform)
				{
					parentTransform->UpdateWorldMatrix();
					m_WorldMatrix = localMat * parentTransform->GetWorldMatrix();
				}
				else
					m_WorldMatrix = localMat;
			}
			else
				m_WorldMatrix = localMat;
		}

		DirectX::SimpleMath::Vector3 wPos, wScale;
		Quaternion wRot;
		m_WorldMatrix.Decompose(wScale, wRot, wPos);

		m_WorldTransform.Position = wPos;
		m_WorldTransform.Quaternion = wRot;
		m_WorldTransform.Scale = wScale;

		m_RightVector = { m_WorldMatrix._11, m_WorldMatrix._12, m_WorldMatrix._13 };
		m_UpVector = { m_WorldMatrix._21, m_WorldMatrix._22, m_WorldMatrix._23 };
		m_ForwardVector = { m_WorldMatrix._31, m_WorldMatrix._32, m_WorldMatrix._33 };

		m_RightVector.Normalize();
		m_UpVector.Normalize();
		m_ForwardVector.Normalize();

		m_bIsTransformDirty = false;
	}

}
