#include "src/Core/Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		PSTR pCmdLine, int nCmdShow)
{
	Blainn::ApplicationDesc AppDesc = {};
	Blainn::Application App(hInstance, AppDesc);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		
	}
}
