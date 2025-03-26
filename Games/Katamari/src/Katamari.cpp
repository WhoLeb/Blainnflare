#include "KatamariLayer.h"

#include "EntryPoint.h"

class KatamariApp : public Blainn::Application
{
public:
	KatamariApp(HINSTANCE instance, const Blainn::ApplicationDesc& appDesc)
		: Blainn::Application(instance, appDesc)
	{}

	void OnInit() override
	{
		m_LayerStack.PushLayer(new KatamariLayer());
	}
};

Blainn::Application* Blainn::CreateApplication(HINSTANCE hInstance)
{
	Blainn::ApplicationDesc appDesc{};
	return new KatamariApp(hInstance, appDesc);
}