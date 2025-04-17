#include "pch.h"
#include "DXSceneVisitor.h"

#include "Core/Camera.h"
#include "DX12/CascadeShadowMaps.h"
#include "EffectPSO.h"

#include <dx12lib/CommandList.h>
#include <dx12lib/Material.h>
#include <dx12lib/Mesh.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SceneNode.h>

using namespace Blainn;

SceneVisitor::SceneVisitor(dx12lib::CommandList& commandList, EffectPSO& lightingPSO, bool transparent)
	: m_CommandList(commandList)
	, m_LightingPSO(lightingPSO)
	, m_TransparentPass(transparent)
{
}

void Blainn::SceneVisitor::Visit(dx12lib::Scene& scene)
{
}

void Blainn::SceneVisitor::Visit(dx12lib::SceneNode& sceneNode)
{
	DirectX::SimpleMath::Matrix world = sceneNode.GetWorldTransform();
	world = world.Transpose();

	m_LightingPSO.SetWorldMatrix(world);
}

void Blainn::SceneVisitor::Visit(dx12lib::Mesh& mesh)
{
	auto material = mesh.GetMaterial();
	if (material->IsTransparent() == m_TransparentPass)
	{
		m_LightingPSO.SetMaterial(material);
		m_LightingPSO.Apply(m_CommandList);
		mesh.Draw(m_CommandList);
	}
}

ShadowVisitor::ShadowVisitor(dx12lib::CommandList& commandList, ShadowMapPSO& shadowPSO)
	: m_CommandList(commandList)
	, m_ShadowPSO(shadowPSO)
{
}

void ShadowVisitor::Visit(dx12lib::Scene& scene)
{
}

void ShadowVisitor::Visit(dx12lib::SceneNode& sceneNode)
{
	DirectX::SimpleMath::Matrix world = sceneNode.GetWorldTransform();
	world = world.Transpose();

	m_ShadowPSO.SetWorldMatrix(world);
}

void ShadowVisitor::Visit(dx12lib::Mesh& mesh)
{
	m_ShadowPSO.Apply(m_CommandList);
	mesh.Draw(m_CommandList);
}
