#include <Windows.h>
#include <vector>
#include <assert.h>
#include <unordered_map>
#include <dinput.h>
#include <crtdbg.h>

#include "fbxsdk.h"
#include "WICTextureLoader.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "SimpleMath.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

using namespace DirectX::SimpleMath;
using namespace DirectX;

