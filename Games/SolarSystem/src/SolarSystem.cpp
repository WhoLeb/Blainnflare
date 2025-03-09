#include "Core/Application.h"
#include "SolarAppLayer.h"

#include "EntryPoint.h"

namespace solar
{
	class SolarSystemApp : public Blainn::Application
	{
	public:
		SolarSystemApp(HINSTANCE instance, const Blainn::ApplicationDesc& appDesc)
			: Blainn::Application(instance, appDesc)
		{

		}

		void OnInit() override
		{
			m_LayerStack.PushLayer(new SolarAppLayer());
		}

	};

}

Blainn::Application* Blainn::CreateApplication(HINSTANCE hInstance)
{
	Blainn::ApplicationDesc appDesc;
	appDesc.WindowWidth = 800;
	appDesc.WindowHeight = 600;
	appDesc.Name = "Solar System";

	return new solar::SolarSystemApp(hInstance, appDesc);
}