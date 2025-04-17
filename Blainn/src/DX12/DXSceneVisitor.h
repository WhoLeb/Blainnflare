#pragma once

#include <dx12lib/Visitor.h>

namespace dx12lib
{
	class CommandList;
}

namespace Blainn
{
	class Camera;
	class EffectPSO;
	class ShadowMapPSO;

	class SceneVisitor : public dx12lib::Visitor
	{
	public:
		SceneVisitor(
			dx12lib::CommandList& commandList,
			EffectPSO& lightingPSO,
			bool transparent);

		void Visit(dx12lib::Scene& scene) override;
		void Visit(dx12lib::SceneNode& sceneNode) override;
		void Visit(dx12lib::Mesh& mesh) override;

	private:
		dx12lib::CommandList&	m_CommandList;
		EffectPSO&				m_LightingPSO;
		bool					m_TransparentPass;
	};

	class ShadowVisitor : public dx12lib::Visitor
	{
	public:
		ShadowVisitor(
			dx12lib::CommandList& commandList,
			ShadowMapPSO& shadowPSO
		);

		void Visit(dx12lib::Scene& scene) override;
		void Visit(dx12lib::SceneNode& sceneNode) override;
		void Visit(dx12lib::Mesh& mesh) override;

	private:
		dx12lib::CommandList&	m_CommandList;
		ShadowMapPSO&			m_ShadowPSO;
	};
}
