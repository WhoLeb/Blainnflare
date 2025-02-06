#pragma once

#include "Window.h"

#include "string"

namespace Blainn
{

	struct ApplicationDesc
	{
		std::string Name = "Blainnflare";
		uint32_t WindowWidth = 1600, WindowHeight = 800;
		bool WindowDecorated = false;
		bool Fullscreen = false;
		bool VSync = true;
	};

	class Application
	{
	public:
		Application(const ApplicationDesc& description);
		virtual ~Application();

		void Run();
		void Close();

		virtual void OnInit() {};
		virtual void OnShutdown() {};
		virtual void OnUpdate() {};
	};

}
