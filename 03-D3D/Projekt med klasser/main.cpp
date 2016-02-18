#include "Linker.h"
#include "System.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	System* system = new System;

	if (!system)
	{
		return 0;
	}

	system->Run(hInstance, nCmdShow);

	system->ShutDown();
	delete system;
	system = 0;

	return 0;
}
