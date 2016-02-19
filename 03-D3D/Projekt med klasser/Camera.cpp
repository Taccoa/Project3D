#include "Camera.h"
#include "FbxImport.h"
//#include "Terrain.h"

Camera::Camera(){}

Camera::~Camera()
{
	DIKeyboard->Unacquire();		//We release controll over the device
	DIMouse->Unacquire();			//We release controll over the device
	DirectInput->Release();			//Prevents Memory Leaks
}

bool Camera::InitDirectInput(HINSTANCE hInstance)
{
	hr = DirectInput8Create(hInstance,	//This is the handle to the instance of our application
		DIRECTINPUT_VERSION,			//This is the version of the direct input we want to use
		IID_IDirectInput8,				//This is an identifier to the interface of direct input we want to use
		(void**)&DirectInput,			//This is the returned pointer to our direct input object
		NULL);							//This is used for COM aggregation

	hr = DirectInput->CreateDevice(GUID_SysKeyboard,	//We enter the flag for the GUID (Globally Unique Identifiers) device we want to use
		&DIKeyboard,									//We return a pointer to the created device
		NULL);											//COM related

	hr = DirectInput->CreateDevice(GUID_SysMouse,	//We enter the flag for the GUID (Globally Unique Identifiers) device we want to use
		&DIMouse,									//We return a pointer to the created device
		NULL);										//COM related

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);	//Lets us tell the device what kind of input we are expecting
	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);			//Lets us tell the device what kind of input we are expecting

	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);


	return true;
}

void Camera::initCamera()
{
	frameCount++;					//Increases our frame count
	if (GetTime() > 1.0f)			//Calls the GetTime function
	{
		fps = frameCount;
		frameCount = 0;
		StartTimer();				//Calls the StartTimer function
	}

	frameTime = GetFrameTime();		//Stores the result of the GetFrameTime function

	DetectInput(frameTime);			//Calls the DetectInput function
}

void Camera::UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);		//Updates the rotation matrix in pitch and yaw
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);		//Updates the target with the NEW rotation matrix
	camTarget = XMVector3Normalize(camTarget);									//Normalizing

	Matrix RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);								//To keep our camera's forward and right vectors pointing only in the x and z axis

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);		//Transforms the vector using the RotateYTempMatrix
	camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);					//Transforms the vector using the RotateYTempMatrix
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);	//Transforms the vector using the RotateYTempMatrix

	camPosition += moveLeftRight*camRight;										//Calculates the cameras NEW position in the right and left position
	camPosition += moveBackForward*camForward;									//Calculates the cameras NEW position in the back and forward position

	moveLeftRight = 0.0f;														//Resets the movement
	moveBackForward = 0.0f;														//Resets the movement

	camTarget = camPosition + camTarget;										//Adds the position with the target

	fbxPtr->test.camPos = camPosition;
	//terrainPtr->material.camPos = camPosition;

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);					//Stores the NEW View Matrix
}

void Camera::DetectInput(double time)
{
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];	//Holds an array of all the possible keys that can be pressed

	DIKeyboard->Acquire();		//We take controll over the device
	DIMouse->Acquire();			//We take controll over the device

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);				//We check if the mouse has moved

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);	//We check if a key has been pressed

	double speed = 15.0f * time;				//This is the speed our camera will move when we reposition it every frame

	if (keyboardState[DIK_ESCAPE] & 0x80)	//We check if it was the ESCAPE key that was pressed
	{
		PostMessage(hwnd, WM_QUIT, 0, 0);	//Exits the window
	}
	if (keyboardState[DIK_A] & 0x80 || keyboardState[DIK_LEFT] & 0x80)		//We check if it was the A or Left key that was pressed
	{
		moveLeftRight -= speed;				//Moves the camera left
	}
	if (keyboardState[DIK_D] & 0x80 || keyboardState[DIK_RIGHT] & 0x80)		//We check if it was the D or Right key that was pressed
	{
		moveLeftRight += speed;				//Moves the camera right
	}
	if (keyboardState[DIK_W] & 0x80 || keyboardState[DIK_UP] & 0x80)		//We check if it was the W or Up key that was pressed
	{
		moveBackForward += speed;			//Moves the camera forward
	}
	if (keyboardState[DIK_S] & 0x80 || keyboardState[DIK_DOWN] & 0x80)		//We check if it was the S or Down key that was pressed
	{
		moveBackForward -= speed;			//Moves the camera back
	}
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY)) //We check where the mouse are now
	{
		camYaw += mouseLastState.lX * 0.001f;
		camPitch += mouseCurrState.lY * 0.001f;
		mouseLastState = mouseCurrState;
	}

	UpdateCamera();		//Call the UpdateCamera function

	return;
}

void Camera::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);			//Gets the time in counts per second

	countsPerSecond = double(frequencyCount.QuadPart);	//Stores the counts per second

	QueryPerformanceCounter(&frequencyCount);			//Gets the current time in counts
	CounterStart = frequencyCount.QuadPart;				//Stores the start of the count
}

double Camera::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);				//Gets the current time in counts
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

double Camera::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);				//Gets the current time in counts

	tickCount = currentTime.QuadPart - frameTimeOld;	//Stores the time it took from the last frame to this frame
	frameTimeOld = currentTime.QuadPart;				//Stores this frame as the next last frame

	if (tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount) / countsPerSecond;
}