#ifndef ENGINE_H
#define ENGINE_H

#include "Linker.h"
class Camera;

class Engine
{

private:
	

public:
	Engine();
	~Engine();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	HRESULT CreateDirect3DContext(HWND wndHandle);

	void CreateShaders();
	void CreateDepthBuffer();
	
	void CreateConstantBuffer();
	void UpdateConstantBuffer();

	void SetViewPort();
	void Render();

	struct VS_CONSTANT_BUFFER
	{
		Matrix worldViewProj;
		Matrix world;
	};

	Camera* cameraPtr;
	Matrix projection;

	//This have to be in public, because it requires direct access in the System Class function "Run".
	IDXGISwapChain* gSwapChain = nullptr; 

	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;
	ID3D11RenderTargetView* gBackbufferRTV = nullptr;

	ID3D11DepthStencilView* gDepthview = nullptr;
	ID3D11Texture2D* gDepthStencilBuffer = nullptr;

	ID3D11Buffer* gVertexBuffer = nullptr;
	ID3D11Buffer* gConstantBuffer = nullptr;

	ID3D11InputLayout* gVertexLayout = nullptr;
	ID3D11VertexShader* gVertexShader = nullptr;
	ID3D11PixelShader* gPixelShader = nullptr;

	//ID3D11GeometryShader* gGeometryShader = nullptr;
};

#endif
