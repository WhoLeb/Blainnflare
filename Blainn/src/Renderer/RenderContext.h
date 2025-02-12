#pragma once


namespace Blainn
{
	class Window;

	class RenderContext
	{
	public:
		RenderContext() = default;
		virtual ~RenderContext() = default;

		virtual void Init(Window wnd) = 0;

		static std::shared_ptr<RenderContext> Create();
	};

}
