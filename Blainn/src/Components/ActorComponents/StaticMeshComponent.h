#pragma once

#include "Components/Component.h"

#include "DX12/DXModel.h"

#include <filesystem>

namespace Blainn
{

	class StaticMeshComponent : public Component
	{
		friend class Scene;

	public:
		StaticMeshComponent(std::filesystem::path filepath)
		{
			m_Model = (std::make_shared<DXModel>(filepath));
		}
		StaticMeshComponent(std::shared_ptr<DXModel> model)
			: m_Model(model)
		{}


		void OnRender() { m_Model->Render(); };

		// internal use for constant buffers
		void SetBufferIndex(UINT32 index) { m_ObjectConstantBufferIndex = index; }
		UINT32 GetBufferIndex() const { return m_ObjectConstantBufferIndex; }

	private:
		std::shared_ptr<DXModel> m_Model;

		UINT32 m_ObjectConstantBufferIndex = -1;
	};
}
