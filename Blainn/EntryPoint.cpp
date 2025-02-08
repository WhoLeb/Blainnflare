#include "src/Core/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		PSTR pCmdLine, int nCmdShow)
{

	try
	{
		if (!Blainn::Application::Initialize(hInstance))
			return -1;
		Blainn::Application& app = Blainn::Application::Get();
		return app.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR failed", MB_OK);
		return -1;
	}
}
