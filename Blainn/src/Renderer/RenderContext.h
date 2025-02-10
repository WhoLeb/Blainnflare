#pragma once

namespace Blainn
{
	class RendererContext
	{
	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;

		virtual void Init() = 0;

		static std::shared_ptr<RendererContext> Create();
	};

}
