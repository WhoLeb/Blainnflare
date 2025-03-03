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
		m_NumFramesDirty = g_NumFrameResources;
		m_Transform.position = newPos;
		m_WorldMatrix =
			DirectX::SimpleMath::Matrix::CreateWorld(newPos, m_ForwardVector, m_UpVector);
	}

	void Actor::SetWorldRotation(const DirectX::SimpleMath::Vector3& newRot)
	{
		m_NumFramesDirty = g_NumFrameResources;
		m_Transform.rotation = newRot;
	}

	void Actor::SetScale(const DirectX::SimpleMath::Vector3& newScale)
	{
		m_NumFramesDirty = g_NumFrameResources;
		m_Transform.scale = newScale;
	}

	void Actor::SetTransform(const Transform& newTransform)
	{
		m_NumFramesDirty = g_NumFrameResources;
		m_Transform = newTransform;
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

}

