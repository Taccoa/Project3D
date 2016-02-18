#ifndef SYSTEM_H
#define SYSTEM_H

#include "Linker.h"
#include "Engine.h"
#include "FbxImport.h"
#include "Camera.h"
#include "Terrain.h"
class Engine;
class FBX;
class Camera;
class Terrain;

class System
{
private:

public:

	System();
	~System();

	int WINAPI Run(HINSTANCE wndHandle, int nCmdShow);
	void ShutDown();

	HWND InitWindow(HINSTANCE hInstance);

	Engine* enginePtr;
	FBX* fbxPtr;
	Camera* cameraPtr;
	Terrain* terrainPtr;

};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#endif 



