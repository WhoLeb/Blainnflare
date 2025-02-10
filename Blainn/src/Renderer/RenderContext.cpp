#include "pch.h"
#include "RenderContext.h"

#include "DX12/DXContext.h"

namespace Blainn
{

	std::shared_ptr<RendererContext> RendererContext::Create()
	{
		return std::make_shared<DXContext>();
	}

}
