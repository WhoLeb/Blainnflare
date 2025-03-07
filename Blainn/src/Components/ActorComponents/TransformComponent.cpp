#include "pch.h"
#include "TransformComponent.h"

extern const int g_NumFrameResources;

namespace Blainn
{
	void TransformComponent::SetPosition(const DirectX::SimpleMath::Vector3& newPos)
	{
		m_Transform.Position = newPos;
		UpdateWorldMatrix();
	}

	void TransformComponent::SetRotation(const DirectX::SimpleMath::Vector3& newRot)
	{
		m_Transform.Rotation = newRot;
		UpdateWorldMatrix();
	}

	void TransformComponent::SetScale(const DirectX::SimpleMath::Vector3& newScale)
	{
		m_Transform.Scale = newScale;
		UpdateWorldMatrix();
	}

	void TransformComponent::UpdateWorldMatrix()
	{
		using namespace DirectX;
		using namespace SimpleMath;
		m_NumFramesDirty = g_NumFrameResources;
		m_WorldMatrix = Matrix::CreateScale(m_Transform.Scale) *
			Matrix::CreateRotationX(m_Transform.Rotation.x) *
			Matrix::CreateRotationY(m_Transform.Rotation.y) *
			Matrix::CreateRotationZ(m_Transform.Rotation.z) *
			Matrix::CreateTranslation(m_Transform.Position);
	}

}
