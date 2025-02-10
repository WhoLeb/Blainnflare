#include "Core/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		PSTR pCmdLine, int nCmdShow)
{
	try
	{
		Blainn::Application app(hInstance);
		if (!app.Initialize())
			return -1;
		return app.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR failed", MB_OK);
		return -1;
	}
}
