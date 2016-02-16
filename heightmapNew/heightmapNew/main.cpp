#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include "bth_image.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "Terrain.h"
//#include "WICTextureLoader.h"

typedef std::vector<int>int_vec_t;

using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace std;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);

IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
ID3D11RenderTargetView* gBackbufferRTV = nullptr;

ID3D11DepthStencilView* gDepthview = nullptr;
ID3D11Texture2D* gDepthStencilBuffer = nullptr;

ID3D11ShaderResourceView* gTextureView = nullptr;
ID3D11Texture2D *gTexture = NULL;

ID3D10ShaderResourceView* hTextureView;
ID3D11Texture2D *hTexture = NULL;

ID3D11Buffer* gVertexBuffer = nullptr;
ID3D11Buffer* gConstantBuffer = nullptr;
ID3D11Buffer* gIndexBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;
ID3D11GeometryShader* gGeometryShader = nullptr;
//------------------------------------------
struct Vertex
{
	Vertex() {};
	Vertex(float x, float y, float z,
		float u, float v, float nx,
		float ny, float nz)
		:pos(x, y, z), texCoord(u, v), nor(nx, ny, nz) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 nor;
};
struct HeightMapInfo
{
	int terrainHeight;
	int terrainWidth;
	XMFLOAT3 *heightMap; //array to store terrain's vertex positions
};

int numFaces = 0;
int numVertices = 0;

bool HeightMapLoad(char* filename, HeightMapInfo &heightMInfo);
bool InitScene();


//function that loads a bmp image and stores hm info in the HeightMapInfo structure
bool HeightMapLoad(char* filename, HeightMapInfo &heightMInfo)
{
	FILE *filePointer;					//point to the current position in the file
	BITMAPFILEHEADER bitmapFileHeader;	//structure that contains info about type, size, layout of the file
	BITMAPINFOHEADER bitmapInfoHeader;	//contains info about the image inside the file
	int imageSize, index;				//index = keeps track of current place in the grid 
										//when filling the hmap structure with pos info
	unsigned char height;				//load in the color value for current read texel
										//we need just one value of the RBG because its a grayscale image 

	filePointer = fopen(filename, "rb"); //r = read, b = binary file 
	if (filePointer == NULL)
		return 0;

	//read bitmaps header
	//pointer to a block of memory, size in bytes of each element, number of elements
	//pointer to FILE objekt that specifies an input stream
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePointer);

	//read info header
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePointer);

	//get width and height of the image
	heightMInfo.terrainHeight = bitmapInfoHeader.biHeight;
	heightMInfo.terrainWidth = bitmapInfoHeader.biWidth;

	int rowBytes = bitmapInfoHeader.biWidth * bitmapInfoHeader.biPlanes;
	rowBytes += 3 - (rowBytes - 1) % 4; // Round up to nearest multiple of 4
										//size of the image in bytes. 3 = RGB (byte, byte, byte) for each pixel
										//imageSize = heightMInfo.terrainHeight * rowBytes;

										//array that stores the image data
	unsigned char* bitmapImage = new unsigned char[bitmapInfoHeader.biSizeImage];

	//set the file pointer to the beginning of the image data
	//filePointer = sets the position indicator associated with the stream to a new position
	//bfoffBits = offset, SEEK_SET = beginning of file
	fseek(filePointer, bitmapFileHeader.bfOffBits, SEEK_SET);

	//store image data in bitmapImage
	fread(bitmapImage, 1, bitmapInfoHeader.biSizeImage, filePointer);
	fclose(filePointer);

	//initialize  the hmap array (stores the verices of the terrain)
	heightMInfo.heightMap = new XMFLOAT3[heightMInfo.terrainHeight * heightMInfo.terrainWidth];

	//divide the height by 10 to water down the terrains height, to smooth the terrain 
	float heightFactor = 10.0f;

	//read the image data info into our heightMap array
	for (int j = 0; j < heightMInfo.terrainHeight; j++)
	{
		for (int i = 0; i < heightMInfo.terrainWidth; i++)
		{
			height = bitmapImage[j * rowBytes + i * bitmapInfoHeader.biPlanes];

			index = (heightMInfo.terrainWidth * j) + i;

			heightMInfo.heightMap[index].x = (float)i;
			heightMInfo.heightMap[index].y = (float)height / heightFactor;
			heightMInfo.heightMap[index].z = (float)j;
		}
	}
	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}
bool InitScene()
{
	//the bitmaps filename i being passed to the hmInfo object
	//so it can loaded with the info of the heightmap
	HeightMapInfo hmInfo;
	HeightMapLoad("heightmapImage.bmp", hmInfo);

	//width and length(cols, rows) of the grid(vertices)
	int cols = hmInfo.terrainWidth;
	int rows = hmInfo.terrainHeight;
	int k = 0;

	//Create the grid 
	numVertices = rows * cols; //to get nr of quads
	numFaces = (rows - 1) * (cols - 1) * 2; //mult with 2 to get the nr of triangles, since it's two triangles in each quad
											
	XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float vecX, vecY, vecZ;
	XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	//compute vertex normals (normal averaging)
	XMVECTOR norSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	int facesUsing = 0;
	float tX;
	float tY;
	float tZ;

	//vector to hold all the vertices
	std::vector<Vertex> v(numVertices); 
	//loop throgh each col and row of the grid
	for (DWORD i = 0; i < rows; i++)
	{
		for (DWORD j = 0; j < cols; j++)
		{
			//define the position of each vertex by using hmInfos info
			v[i*cols + j].pos = hmInfo.heightMap[i*cols + j];
			//initializing the normal so it points directly up, faces of the terrain will be lit equally
			//if you have a normal defined in the hmap file, set it here the same way as the position(saves runtime) 
			v[i*cols + j].nor = XMFLOAT3(0.0f, 1.0f, 0.0f);
			v[i*cols + j].texCoord = XMFLOAT2(float(i) / rows, float(j) / cols);

		}
	}

	std::vector<DWORD> indices(numFaces * 3);
	for (DWORD i = 0; i < rows - 1; i++)
	{
		for (DWORD j = 0; j < cols - 1; j++)
		{
			indices[k] = i * cols + j;
			indices[k + 2] = i * cols + j + 1;
			indices[k + 1] = (i + 1) * cols + j;
			indices[k + 3] = (i + 1) * cols + j;
			indices[k + 5] = i * cols + j + 1;
			indices[k + 4] = (i + 1) * cols + j + 1;

			k += 6;
		}
	}

	std::vector<XMFLOAT3> tmpNormal;
	for (int i = 0; i < numFaces; i++)
	{
		//get the vector describing one edge of the triangle (edge 0,2)
		vecX = v[indices[(i * 3)]].pos.x - v[indices[(i * 3) + 2]].pos.x;
		vecY = v[indices[(i * 3)]].pos.y - v[indices[(i * 3) + 2]].pos.y;
		vecZ = v[indices[(i * 3)]].pos.z - v[indices[(i * 3) + 2]].pos.z;
		edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//create first edge 

		//edge(2,1)
		vecX = v[indices[(i * 3) + 2]].pos.x - v[indices[(i * 3) + 1]].pos.x;
		vecY = v[indices[(i * 3) + 2]].pos.y - v[indices[(i * 3) + 1]].pos.y;
		vecZ = v[indices[(i * 3) + 2]].pos.z - v[indices[(i * 3) + 1]].pos.z;
		edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//create second edge 

		//get unnormalized face normal by cross multiply the two edge vectors   
		XMVECTOR faceNormal = XMVector3Cross(edge1, edge2);
		tmpNormal.push_back(unnormalized);	//save unnormalized normal(for normal averaging)
	}

	//go throgh each vertex
	for (int i = 0; i < numVertices; i++)
	{
		//check which triangles use this vertex 
		for (int j = 0; j < numFaces; j++)
		{
			if (indices[j * 3] == i || indices[(j * 3) + 1] == i || indices[(j * 3) + 2] == i)
			{
				tX = XMVectorGetX(norSum) + tmpNormal[j].x;
				tY = XMVectorGetY(norSum) + tmpNormal[j].y;
				tZ = XMVectorGetY(norSum) + tmpNormal[j].z;

				norSum = XMVectorSet(tX, tY, tZ, 0.0f);
				facesUsing++;
			}
		}

		//get normal by dividing the norSum by th nr of faces sharing the vertex
		norSum = norSum / facesUsing;
		norSum = XMVector3Normalize(norSum);

		//store the normal in current vertex
		v[i].nor.x = XMVectorGetX(norSum);
		v[i].nor.y = XMVectorGetX(norSum);
		v[i].nor.z = XMVectorGetX(norSum);

		//clear norSum and facesUsing for next vertex
		norSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		facesUsing = 0;
	}

	D3D11_BUFFER_DESC VertexBufferDesc;
	memset(&VertexBufferDesc, 0, sizeof(VertexBufferDesc));
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.ByteWidth = sizeof(Vertex) * numVertices;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA VertexBufferData;
	memset(&VertexBufferData, 0, sizeof(VertexBufferData));
	VertexBufferData.pSysMem = &v[0];

	gDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferData, &gVertexBuffer);

	D3D11_BUFFER_DESC IndexBufferDesc;
	memset(&IndexBufferDesc, 0, sizeof(IndexBufferDesc));
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.ByteWidth = sizeof(DWORD) * numFaces * 3;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA IndexBufferData;
	memset(&IndexBufferData, 0, sizeof(IndexBufferData));
	IndexBufferData.pSysMem = &indices[0];

	gDevice->CreateBuffer(&IndexBufferDesc, &IndexBufferData, &gIndexBuffer);

	return true;
}
//------------------------------------------
struct VS_CONSTANT_BUFFER
{
	Matrix worldViewProj;
	Matrix world;
}; VS_CONSTANT_BUFFER;

void CreateConstantBuffer()
{
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;


	//Creating the Constant Buffer
	gDevice->CreateBuffer(&desc, NULL, &gConstantBuffer);
}

void UpdateConstantBuffer()
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedCB;
	VS_CONSTANT_BUFFER* dataPtr;

	Matrix world;
	Matrix view;
	Matrix projection;
	Matrix worldViewProjection;

	static float rotation = 0;
	rotation += 0.01f;

	VS_CONSTANT_BUFFER vsCBuffer;

	//world = XMMatrixTranslation(0, 0, 0) * XMMatrixRotationY(XMConvertToRadians(rotation));
	world = XMMatrixScaling(0.018, 0.018, 0.018) * XMMatrixTranslation(-2.3, -0.6, -2.0);

	view = XMMatrixLookAtLH(Vector3(0, 1, -3), Vector3(0, 0, 0), Vector3(0, 1, 0));
	projection = XMMatrixPerspectiveFovLH(float(3.1415 * 0.45), float(640.0 / 480.0), float(0.5), float(20));

	worldViewProjection = world * view * projection;
	worldViewProjection = worldViewProjection.Transpose();

	world = world.Transpose();

	result = gDeviceContext->Map(gConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB);
	if (FAILED(result))
	{
		return;
	}

	dataPtr = (VS_CONSTANT_BUFFER*)mappedCB.pData;
	dataPtr->worldViewProj = worldViewProjection;
	dataPtr->world = world;

	gDeviceContext->Unmap(gConstantBuffer, 0);

	gDeviceContext->GSSetConstantBuffers(0, 1, &gConstantBuffer);
};

void CreateDepthBuffer()
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = (float)640;
	desc.Height = (float)480;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	gDevice->CreateTexture2D(&desc, 0, &gDepthStencilBuffer);

	gDevice->CreateDepthStencilView(gDepthStencilBuffer, 0, &gDepthview);
}

void CreateTexture()
{
	D3D11_TEXTURE2D_DESC bthTexDesc;
	ZeroMemory(&bthTexDesc, sizeof(bthTexDesc));
	bthTexDesc.Width = BTH_IMAGE_WIDTH;
	bthTexDesc.Height = BTH_IMAGE_HEIGHT;
	bthTexDesc.MipLevels = bthTexDesc.ArraySize = 1;
	bthTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bthTexDesc.SampleDesc.Count = 1;
	bthTexDesc.SampleDesc.Quality = 0;
	bthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	bthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bthTexDesc.CPUAccessFlags = 0;
	bthTexDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)BTH_IMAGE_DATA;
	data.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);
	gDevice->CreateTexture2D(&bthTexDesc, &data, &gTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc, sizeof(resViewDesc));
	resViewDesc.Format = bthTexDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	gDevice->CreateShaderResourceView(gTexture, &resViewDesc, &gTextureView);

}

void CreateShaders()
{
	//create vertex shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"Vertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pVS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);

	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
	// we do not need anymore this COM object, so we release it.
	pVS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(
		L"Fragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
						// how to use the Error blob, see here
						// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	// we do not need anymore this COM object, so we release it.
	pPS->Release();

	//create geometry shader
	ID3DBlob* pGS = nullptr;
	D3DCompileFromFile(
		L"Geometry.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"GS_main",		// entry point
		"gs_4_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pGS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		);

	gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	pGS->Release();
}

void CreateTriangleData()
{
	/*struct TriangleVertex
	{
	float x, y, z;
	float u, v;
	};

	TriangleVertex triangleVertices[] =
	{
	-0.5f, 0.5f, 0.0f,	//v0 pos
	0.0f, 0.0f,			//v0 uv

	0.5f, -0.5f, 0.0f,	//v1
	1.0f, 1.0f,			//v1 uv

	-0.5f, -0.5f, 0.0f, //v2
	0.0f, 1.0f,			//v2 uv

	// Triangle 2

	0.5f, -0.5f, 0.0f,	//v0 pos
	1.0f, 1.0f,			//v0 uv

	-0.5f, 0.5f, 0.0f,	//v1
	0.0f, 0.0f,			//v1 uv

	0.5f, 0.5f, 0.0f,   //v2
	1.0f, 0.0f			//v2 uv

	};*/


	/*D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA VertexBufferdata;
	memset(&VertexBufferdata, 0, sizeof(VertexBufferdata));
	VertexBufferdata.pSysMem = v;
	gDevice->CreateBuffer(&bufferDesc, &VertexBufferdata, &gVertexBuffer);*/

}

void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)640;
	vp.Height = (float)480;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

void Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 1, 0, 0, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);

	gDeviceContext->ClearDepthStencilView(gDepthview, D3D11_CLEAR_DEPTH, 1.0f, 0);

	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
	gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);

	UINT32 vertexSize = sizeof(Vertex);
	UINT32 offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetIndexBuffer(gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->IASetInputLayout(gVertexLayout);

	UpdateConstantBuffer();

	gDeviceContext->DrawIndexed(numFaces * 3, 0, 0);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa f�nster
	CoInitialize(NULL);

	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. S�tt viewport

		CreateShaders(); //4. Skapa vertex- och pixel-shaders

		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		CreateConstantBuffer(); //Calls the CreateConstantBuffer function

		CreateTexture();

		InitScene();

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
				Render(); //8. Rendera

				gSwapChain->Present(0, 0); //9. V�xla front- och back-buffer
			}
		}

		gVertexBuffer->Release();
		gConstantBuffer->Release(); //Prevents Memory Leaks
		gDepthStencilBuffer->Release(); //Prevents Memory Leaks
		gIndexBuffer->Release();

		gDepthview->Release(); //Prevents Memory Leaks
		gTextureView->Release(); //Prevents Memory Leaks
		gTexture->Release(); //Prevents Memory Leaks

		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		gGeometryShader->Release(); //Prevents Memory Leaks

		gBackbufferRTV->Release();
		gSwapChain->Release();
		gDevice->Release();
		gDeviceContext->Release();
		DestroyWindow(wndHandle);
	}

	return (int)msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = "BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindowW(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
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

HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		CreateDepthBuffer(); //Calls the CreateDepthBuffer function

							 // set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, gDepthview);
	}
	return hr;
}