#include "pch.h"
#include "Actor.h"

#include "DX12/DXGraphicsPrimitive.h"

namespace Blainn
{

	//void Actor::OnRender()
	//{
	//	
	//}

	void Actor::SetModel(std::shared_ptr<DXGraphicsPrimitive> model)
	{
		m_GraphicsPrimitive = model;
	}

	void Actor::SetWorldPosition(const DirectX::SimpleMath::Vector3& newPos)
	{
		m_Transform.position = newPos;
		UpdateWorldMatrix();
	}

	void Actor::SetWorldRotation(const DirectX::SimpleMath::Vector3& newRot)
	{
		m_NumFramesDirty = g_NumFrameResources;
		m_Transform.rotation = newRot;

		// TODO: rotate forward and up vector accordingly

		UpdateWorldMatrix();
	}

	void Actor::SetForwardVector(const DirectX::SimpleMath::Vector3& newForwardVector)
	{
		newForwardVector.Normalize(m_ForwardVector);
		UpdateWorldMatrix();
	}

	void Actor::SetUpVector(const DirectX::SimpleMath::Vector3& newUpVector)
	{
		newUpVector.Normalize(m_UpVector);
		UpdateWorldMatrix();
	}

	void Actor::SetScale(const DirectX::SimpleMath::Vector3& newScale)
	{
		m_Transform.scale = newScale;
		UpdateWorldMatrix();
	}

	void Actor::SetTransform(const Transform& newTransform)
	{
		m_Transform = newTransform;
		UpdateWorldMatrix();
	}

	void Actor::SetWorldMatrix(const DirectX::SimpleMath::Matrix& newWorldMatrix)
	{
		m_NumFramesDirty = g_NumFrameResources;
		m_WorldMatrix = newWorldMatrix;
	}

	void Actor::OnRender()
	{
		m_GraphicsPrimitive->Draw();
	}

	void Actor::UpdateWorldMatrix()
	{
		m_NumFramesDirty = g_NumFrameResources;

		m_WorldMatrix = DirectX::SimpleMath::Matrix::CreateScale(m_Transform.scale);
		m_WorldMatrix *= DirectX::SimpleMath::Matrix::CreateWorld(m_Transform.position, m_ForwardVector, m_UpVector);
	}

}

