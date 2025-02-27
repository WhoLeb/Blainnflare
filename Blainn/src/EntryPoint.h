#pragma once

#include "Core/Application.h"

#if defined DEBUG || defined _DEBUG
#include <iostream>
#include <fstream>
#endif // 

extern Blainn::Application* Blainn::CreateApplication(HINSTANCE hInstance);
bool g_ApplicationRunning = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		PSTR pCmdLine, int nCmdShow)
{
#if defined DEBUG || defined _DEBUG
	AllocConsole();

	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "w", stdin);

	std::cout << "Debug console initialized!\n";
#endif // 

	try
	{
		Blainn::Application* app = Blainn::CreateApplication(hInstance);
		if (!app || !app->Initialize())
			return -1;
		return app->Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR failed", MB_OK);
		return -1;
	}
}
