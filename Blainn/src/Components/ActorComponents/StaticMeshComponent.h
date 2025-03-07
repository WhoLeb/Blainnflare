#pragma once

#include "Components/Component.h"

#include <filesystem>

namespace Blainn
{
	class DXModel;

	class StaticMeshComponent : public Component
	{
		StaticMeshComponent(std::filesystem::path filepath);
		StaticMeshComponent(std::shared_ptr<DXModel> model);
	public:

		void OnRender();

		void SetBufferIndex(UINT32 index) { m_ObjectConstantBufferIndex = index; }
		UINT32 GetBufferIndex() const { return m_ObjectConstantBufferIndex; }


	private:
		std::shared_ptr<DXModel> m_Model;

		UINT32 m_ObjectConstantBufferIndex = -1;
	};
}
