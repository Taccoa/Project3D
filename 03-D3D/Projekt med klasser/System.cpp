#include "Linker.h"
#include "Camera.h"
#include "FbxImport.h"
#include "Terrain.h"
#include "Engine.h"
#include "System.h"
#include "Frustum.h"

System::System()
{
}

System::~System()
{
}

int WINAPI System::Run(HINSTANCE wHandle, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(wHandle);

	enginePtr = new Engine;
	fbxPtr = new FBX;
	cameraPtr = new Camera;
	terrainPtr = new Terrain;
	frustumPtr = new Frustum;


	if (!cameraPtr->InitDirectInput(wHandle)) //We call our function and controlls that it does load
	{
		MessageBox(0, L"Direct Input Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	fbxPtr->systemPtr = this;
	enginePtr->cameraPtr = cameraPtr;

	cameraPtr->fbxPtr = fbxPtr;

	terrainPtr->enginePtr = enginePtr;
	terrainPtr->cameraPtr = cameraPtr;

	frustumPtr->cameraPtr = cameraPtr;
	frustumPtr->enginePtr = enginePtr;

	if (wndHandle)
	{
		CoInitialize(NULL);

		enginePtr->CreateDirect3DContext(wndHandle);
		enginePtr->CreateConstantBuffer();
		enginePtr->SetViewPort();
		enginePtr->CreateShaders();

		fbxPtr->InitializeModels();

		terrainPtr->InitScene();

		terrainPtr->CreateHeightTexture();

		//--------------------------------------
		terrainPtr->CreateTerrainMatrixBuffer();
		terrainPtr->createTerrainMaterialBuffer();
		
		//--------------------------------------

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			else
			{
				enginePtr->UpdateConstantBuffer();
				enginePtr->Render();

				fbxPtr->RenderFBX();
				
				terrainPtr->UpdateTerrainMatrixBuffer();
				terrainPtr->updateTerrainMaterialBuffer();
				terrainPtr->RenderTerrain();

				frustumPtr->getFrustumPlanes();
				cameraPtr->initCamera();
				enginePtr->gSwapChain->Present(1, 0);
			}
		}

		DestroyWindow(wndHandle);
	}
	
	return (int)msg.wParam;
}

void System::ShutDown()
{
	if (enginePtr)
	{
		delete enginePtr;
		enginePtr = 0;
	}

	if (fbxPtr)
	{
		delete fbxPtr;
		fbxPtr = 0;
	}

	if (cameraPtr)
	{
		delete cameraPtr;
		cameraPtr = 0;
	}

	if (terrainPtr)
	{
		delete terrainPtr;
		terrainPtr = 0;
	}
	if (frustumPtr)
	{
		delete frustumPtr;
		frustumPtr = 0;
	}

	return;
}

HWND System::InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"DIRECTX_PROJECT";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 1280, 960 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"DIRECTX_PROJECT",
		L"DirectX Project",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
