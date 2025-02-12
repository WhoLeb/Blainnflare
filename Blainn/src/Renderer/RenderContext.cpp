#include "pch.h"
#include "RenderContext.h"

#include "DX12/DXContext.h"

namespace Blainn
{

	std::shared_ptr<RenderContext> RenderContext::Create()
	{
		return std::make_shared<DXContext>();
	}

}
