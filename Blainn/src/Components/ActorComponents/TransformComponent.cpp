#include "pch.h"
#include "TransformComponent.h"

#include "Core/GameObject.h"

#include <iostream>

extern const int g_NumFrameResources;

namespace Blainn
{
	TransformComponent::TransformComponent(std::shared_ptr<GameObject> owner)
		: Super(owner)
	{
	}

	void TransformComponent::OnAttach()
	{
		Super::OnAttach();
		MarkDirty();
	}

	void TransformComponent::OnUpdate(const GameTimer& gt)
	{
		Super::OnUpdate(gt);
		UpdateWorldMatrix();
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
		float pitchRad = XMConvertToRadians(newLocalRot.y);
		float rollRad = XMConvertToRadians(newLocalRot.z);

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
		SetLocalPosition(newLocalPos);
		SetLocalYawPitchRoll(newLocalRot);
	}

	void TransformComponent::SetLocalPositionQuat(const DirectX::SimpleMath::Vector3& newLocalPos, const DirectX::SimpleMath::Quaternion& newLocalQuat)
	{
		SetLocalPosition(newLocalPos);
		SetLocalQuat(newLocalQuat);
	}

	void TransformComponent::SetWorldPosition(const DirectX::SimpleMath::Vector3& newWorldPos)
	{
		using namespace DirectX::SimpleMath;

		MarkDirty();

		auto owner = GetOwner();
		auto parentObj = owner ? owner->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Position = newWorldPos;
			return;
		}

		// Else, invert parent's world matrix:
		auto parentTransform = parentObj->GetComponent<TransformComponent>();
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
		float pitchRad = XMConvertToRadians(newWorldRot.y);
		float rollRad = XMConvertToRadians(newWorldRot.z);

		Quaternion worldQ = Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
		worldQ.Normalize();

		SetWorldQuat(worldQ);
	}

	void TransformComponent::SetWorldQuat(const DirectX::SimpleMath::Quaternion& newWorldQuat)
	{
		using namespace DirectX::SimpleMath;
		MarkDirty();

		// If no parent, local=world:
		auto owner = GetOwner();
		auto parentObj = owner ? owner->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Quaternion = newWorldQuat;
			m_LocalTransform.Quaternion.Normalize();
			m_WorldTransform.Quaternion = newWorldQuat;
			m_WorldTransform.Quaternion.Normalize();
			return;
		}

		auto parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Quaternion = newWorldQuat;
			m_LocalTransform.Quaternion.Normalize();
			m_WorldTransform.Quaternion = newWorldQuat;
			m_WorldTransform.Quaternion.Normalize();
			return;
		}

		Quaternion parentWorldRot = parentTransform->GetWorldQuat();
		parentWorldRot.Normalize();

		// local = conj(parentRot) * worldRot
		parentWorldRot.Conjugate();
		Quaternion local = parentWorldRot * newWorldQuat;
		local.Normalize();

		m_LocalTransform.Quaternion = local;
		m_WorldTransform.Quaternion = local;
	}

	void TransformComponent::SetWorldScale(const DirectX::SimpleMath::Vector3& newWorldScale)
	{
		using namespace DirectX::SimpleMath;

		MarkDirty();

		auto owner = GetOwner();
		auto parentObj = owner ? owner->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Scale = newWorldScale;
			return;
		}

		auto parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Scale = newWorldScale;
			return;
		}

		Vector3 parentWorldScale = parentTransform->GetWorldScale();
		m_LocalTransform.Scale = newWorldScale / parentWorldScale;
	}

	void TransformComponent::SetWorldPositionYawPitchRoll(const DirectX::SimpleMath::Vector3& newWorldPos, const DirectX::SimpleMath::Vector3& newWorldRot)
	{
		SetWorldPosition(newWorldPos);
		SetWorldYawPitchRoll(newWorldRot);
	}

	void TransformComponent::SetWorldPositionQuat(const DirectX::SimpleMath::Vector3& newWorldPos, const DirectX::SimpleMath::Quaternion& newWorldQuat)
	{
		SetWorldPosition(newWorldPos);
		SetWorldQuat(newWorldQuat);
	}

	DirectX::SimpleMath::Vector3 TransformComponent::GetWorldYawPitchRoll() const
	{
		using namespace DirectX;
		auto q = m_WorldTransform.Quaternion;

		DirectX::SimpleMath::Vector3 eulerAngles = m_WorldTransform.Quaternion.ToEuler();

		float yaw = XMConvertToDegrees(eulerAngles.y);
		float pitch = XMConvertToDegrees(eulerAngles.x);
		float roll = XMConvertToDegrees(eulerAngles.z);
		return DirectX::SimpleMath::Vector3(yaw, pitch, roll);
	}

	DirectX::SimpleMath::Vector3 TransformComponent::GetLocalYawPitchRoll() const
	{
		using namespace DirectX;
		DirectX::SimpleMath::Vector3 eulerAngles = m_LocalTransform.Quaternion.ToEuler();

		float yaw = XMConvertToDegrees(eulerAngles.y);
		float pitch = XMConvertToDegrees(eulerAngles.x);
		float roll = XMConvertToDegrees(eulerAngles.z);
		return DirectX::SimpleMath::Vector3(yaw, pitch, roll);
	}

	void TransformComponent::MarkDirty()
	{
		if (!m_bIsTransformDirty)
		{
			m_bIsTransformDirty = true;

			if (auto owner = GetOwner())
			{
				for (auto& child : owner->GetChildren())
				{
					auto childTransform = child->GetComponent<TransformComponent>();
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

		if (auto owner = GetOwner())
		{
			auto parentObj = owner->GetParent();
			if (parentObj)
			{
				auto parentTransform = parentObj->GetComponent<TransformComponent>();
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
		else
			return;

		DirectX::SimpleMath::Vector3 wPos, wScale;
		Quaternion wRot;
		m_WorldMatrix.Decompose(wScale, wRot, wPos);

		m_WorldTransform.Position = wPos;
		m_WorldTransform.Quaternion = wRot;
		m_WorldTransform.Scale = wScale;

		//m_RightVector   = Vector3::Transform(Vector3::UnitX, m_WorldMatrix);
		//m_UpVector      = Vector3::Transform(Vector3::UnitY, m_WorldMatrix);
		//m_ForwardVector = Vector3::Transform(Vector3::UnitZ, m_WorldMatrix);

		m_RightVector   = { m_WorldMatrix._11, m_WorldMatrix._12, m_WorldMatrix._13 };
		m_UpVector      = { m_WorldMatrix._21, m_WorldMatrix._22, m_WorldMatrix._23 };
		m_ForwardVector = { m_WorldMatrix._31, m_WorldMatrix._32, m_WorldMatrix._33 };

		m_RightVector.Normalize();
		m_UpVector.Normalize();
		m_ForwardVector.Normalize();

		m_bIsTransformDirty = false;
	}

}
