#ifndef CAMERA_H
#define CAMERA_H

#include "Linker.h"
class FBX;
//class Terrain;

class Camera
{
private:

public:
	Camera();
	~Camera();

	Vector4 DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	Vector4 camPosition = XMVectorSet(0.0f, 2.0f, -10.f, 0.0f);
	Vector4 camTarget = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
	Vector4 camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	Matrix camRotationMatrix;
	Matrix camView;

	double moveLeftRight = 0.0f;
	double moveBackForward = 0.0f;

	double camYaw = 0.0f;
	double camPitch = 0.0f;

	void UpdateCamera();

	IDirectInputDevice8* DIKeyboard;
	IDirectInputDevice8* DIMouse;

	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	bool InitDirectInput(HINSTANCE hInstance);
	void DetectInput(double time);

	HRESULT hr;
	HWND hwnd = NULL;

	double countsPerSecond = 0.0;
	__int64 CounterStart = 0;

	int frameCount = 0;
	int fps = 0;

	__int64 frameTimeOld = 0;
	double frameTime;

	void StartTimer();
	double GetTime();
	double GetFrameTime();

	void initCamera();

	FBX* fbxPtr;
	//Terrain* terrainPtr;
};

#endif CAMERA_H