#include "pch.h"
#include "DXFrameResource.h"

#include "Core/Application.h"

namespace Blainn
{
	DXFrameResource::DXFrameResource(UINT passCount, UINT objectCount)
	{
		auto& device = Application::Get().GetRenderingContext()->GetDevice()->Device();
		ThrowIfFailed(device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&m_CmdListAllocator)
		));
		auto& resourceManager = Application::Get().GetResourceManager();

		m_PassConstantBuffer = std::make_unique<DXUploadBuffer<PassConstants>>(resourceManager, passCount, true);
		m_ObjectsConstantBuffer = std::make_unique<DXUploadBuffer<ObjectConstants>>(resourceManager, objectCount, true);
	}
}

