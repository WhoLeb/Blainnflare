#include "pch.h"
#include "TransformComponent.h"

#include "Core/GameObject.h"

extern const int g_NumFrameResources;

namespace Blainn
{
	void TransformComponent::SetLocalPosition(const DirectX::SimpleMath::Vector3& newLocalPos)
	{
		m_LocalTransform.Position = newLocalPos;
		UpdateWorldMatrix();
	}

	void TransformComponent::SetLocalYawPitchRoll(const DirectX::SimpleMath::Vector3& newLocalRot)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		float yawRad = XMConvertToRadians(newLocalRot.y);
		float pitchRad = XMConvertToRadians(newLocalRot.x);
		float rollRad = XMConvertToRadians(newLocalRot.z);

		Quaternion localRot = Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
		localRot.Normalize();
		m_LocalTransform.Quaternion = localRot;

		UpdateWorldMatrix();
	}

	void TransformComponent::SetLocalQuat(const DirectX::SimpleMath::Quaternion& newLocalRotQuat)
	{
		m_LocalTransform.Quaternion = newLocalRotQuat;
		m_LocalTransform.Quaternion.Normalize();
		UpdateWorldMatrix();
	}

	void TransformComponent::SetLocalScale(const DirectX::SimpleMath::Vector3& newLocalScale)
	{
		m_LocalTransform.Scale = newLocalScale;
		UpdateWorldMatrix();
	}

	void TransformComponent::SetLocalPositionYawPitchRoll(const DirectX::SimpleMath::Vector3& newLocalPos, const DirectX::SimpleMath::Vector3& newLocalRot)
	{
		m_LocalTransform.Position = newLocalPos;
		m_LocalTransform.Quaternion = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(newLocalRot);
		UpdateWorldMatrix();
	}

	void TransformComponent::SetLocalPositionQuat(const DirectX::SimpleMath::Vector3& newLocalPos, const DirectX::SimpleMath::Quaternion& newLocalQuat)
	{
		m_LocalTransform.Position = newLocalPos;
		m_LocalTransform.Quaternion = newLocalQuat;
		m_LocalTransform.Quaternion.Normalize();
		UpdateWorldMatrix();
	}

	void TransformComponent::SetWorldPosition(const DirectX::SimpleMath::Vector3& newWorldPos)
	{
		using namespace DirectX::SimpleMath;

		auto parentObj = GetOwner() ? GetOwner()->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Position = newWorldPos;
			UpdateWorldMatrix();
			return;
		}

		// Else, invert parent's world matrix:
		auto* parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Position = newWorldPos;
			UpdateWorldMatrix();
			return;
		}
		Matrix invParent = parentTransform->GetWorldMatrix().Invert();

		// Convert world => local
		Vector3 localPos = Vector3::Transform(newWorldPos, invParent);
		m_LocalTransform.Position = localPos;

		UpdateWorldMatrix();
	}

	void TransformComponent::SetWorldYawPitchRoll(const DirectX::SimpleMath::Vector3& newWorldRot)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		float pitchRad = XMConvertToRadians(newWorldRot.x);
		float yawRad = XMConvertToRadians(newWorldRot.y);
		float rollRad = XMConvertToRadians(newWorldRot.z);

		Quaternion worldQ = Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
		worldQ.Normalize();

		TransformComponent* parentTransform = GetOwner() ? GetOwner()->GetComponent<TransformComponent>() : nullptr;
		if (parentTransform)
		{
			Quaternion parentWorldRot = parentTransform->GetWorldQuat();
			parentWorldRot.Normalize();

			parentWorldRot.Conjugate();
			Quaternion localQ = parentWorldRot * worldQ;

			m_LocalTransform.Quaternion = localQ;
		}
		else
			m_LocalTransform.Quaternion = worldQ;

		UpdateWorldMatrix();
	}

	void TransformComponent::SetWorldQuat(const DirectX::SimpleMath::Quaternion& newWorldQuat)
	{
		using namespace DirectX::SimpleMath;

		// If no parent, local=world:
		auto parentObj = GetOwner() ? GetOwner()->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Quaternion = newWorldQuat;
			m_LocalTransform.Quaternion.Normalize();
			UpdateWorldMatrix();
			return;
		}

		auto* parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Quaternion = newWorldQuat;
			m_LocalTransform.Quaternion.Normalize();
			UpdateWorldMatrix();
			return;
		}

		Quaternion parentWorldRot = parentTransform->GetWorldQuat();
		parentWorldRot.Normalize();

		// local = conj(parentRot) * worldRot
		parentWorldRot.Conjugate();
		Quaternion local = parentWorldRot * newWorldQuat;
		local.Normalize();

		m_LocalTransform.Quaternion = local;

		UpdateWorldMatrix();
	}

	void TransformComponent::SetWorldScale(const DirectX::SimpleMath::Vector3& newWorldScale)
	{
		using namespace DirectX::SimpleMath;

		auto parentObj = GetOwner() ? GetOwner()->GetParent() : nullptr;
		if (!parentObj) {
			m_LocalTransform.Scale = newWorldScale;
			UpdateWorldMatrix();
			return;
		}

		auto* parentTransform = parentObj->GetComponent<TransformComponent>();
		if (!parentTransform) {
			m_LocalTransform.Scale = newWorldScale;
			UpdateWorldMatrix();
			return;
		}

		Vector3 parentWorldScale = parentTransform->GetWorldScale();
		m_LocalTransform.Scale = newWorldScale / parentWorldScale;

		UpdateWorldMatrix();
	}

	void TransformComponent::SetWorldPositionYawPitchRoll(const DirectX::SimpleMath::Vector3& newWorldPos, const DirectX::SimpleMath::Vector3& newWorldRot)
	{
	}

	void TransformComponent::SetWorldPositionQuat(const DirectX::SimpleMath::Vector3& newWorldPos, const DirectX::SimpleMath::Quaternion& newWorldQuat)
	{
	}

	void TransformComponent::UpdateWorldMatrix()
	{
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
					m_WorldMatrix = parentTransform->GetWorldMatrix() * localMat;
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

		m_RightVector =		{ m_WorldMatrix._11, m_WorldMatrix._12, m_WorldMatrix._13 };
		m_UpVector =		{ m_WorldMatrix._21, m_WorldMatrix._22, m_WorldMatrix._23 };
		m_ForwardVector =	{ m_WorldMatrix._31, m_WorldMatrix._32, m_WorldMatrix._33 };

		m_RightVector.Normalize();
		m_UpVector.Normalize();
		m_ForwardVector.Normalize();
	}

}
