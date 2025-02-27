#include "Core/Application.h"

#include "EntryPoint.h"

namespace Pong
{
	class PongApplication : public Blainn::Application
	{
	public:
		PongApplication(HINSTANCE hInstance, const Blainn::ApplicationDesc& appDesc)
			: Application(hInstance)
		{

		}

		void OnInit() {}

	private:

	};
}

Blainn::Application* Blainn::CreateApplication(HINSTANCE hInstance)
{
	ApplicationDesc appDesc{};
	appDesc.Fullscreen = false;
	appDesc.Name = "Pong";
	appDesc.WindowHeight = 600;
	appDesc.WindowWidth = 800;
	
	return new Pong::PongApplication(hInstance, appDesc);
}