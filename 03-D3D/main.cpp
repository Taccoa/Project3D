
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "SimpleMath.h"
#include "bth_image.h"

#include <fbxsdk.h>
#include <vector>
#include <assert.h>

using namespace DirectX::SimpleMath;
using namespace DirectX;

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

ID3D11Buffer* gVertexBuffer = nullptr;
ID3D11Buffer* gConstantBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;
//ID3D11GeometryShader* gGeometryShader = nullptr;

int vertexVector = 0;

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

	world = XMMatrixTranslation(0, 0, 0) * XMMatrixRotationY(XMConvertToRadians(rotation));
	view = XMMatrixLookAtLH(Vector3(0, 0, -2), Vector3(0, 0, 0), Vector3(0, 1, 0));
	projection = XMMatrixPerspectiveFovLH(float(3.1415*0.45), float(640.0 / 480.0), float(0.5), float(20));

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

	gDeviceContext->VSSetConstantBuffers(0, 1, &gConstantBuffer);
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
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0   },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

	////create geometry shader
	//ID3DBlob* pGS = nullptr;
	//D3DCompileFromFile(
	//	L"Geometry.hlsl", // filename
	//	nullptr,		// optional macros
	//	nullptr,		// optional include files
	//	"GS_main",		// entry point
	//	"gs_4_0",		// shader model (target)
	//	0,				// shader compile options
	//	0,				// effect compile options
	//	&pGS,			// double pointer to ID3DBlob		
	//	nullptr			// pointer for Error Blob messages.
	//	);

	//gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	//pGS->Release();
}

struct FBXData
{
	float pos[3];
	int nor[3];
	float uv[2];
};

void InitiSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
	pManager = FbxManager::Create(); //Creates the Manager for FBX and is object allocator for almost all classes.

	if (!pManager) //If the manager isn't initialized.
	{
		FBXSDK_printf("Error: Can't create FBX Manager!\n");
		exit(1); //Exit the application.
	}

	else
	{
		FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT); //Creates IO Settings object. Holds the settings for import/export settings.
		pManager->SetIOSettings(ios);

		pScene = FbxScene::Create(pManager, "Test Scene"); //Creates the scene. Object hold other objects imported from files.

		if (!pScene) //If scene isn't initialized.
		{
			FBXSDK_printf("Error: Can't create FBX Scene\n");
			exit(1); //Exit the application.
		}
	}
}

FbxMesh* LoadScene(FbxManager* pManager, FbxScene* pScene)
{
	FbxImporter* myImporter = FbxImporter::Create(pManager, "My importer"); //Creates the importer to use with SDK.

	FbxMesh* myMesh = nullptr;

	bool importStatus = myImporter->Initialize("D:/test3.fbx", -1, pManager->GetIOSettings()); //Initialize the importer with a filename.
	
	if (!importStatus) //If the importer can't be initialized.
	{
		FBXSDK_printf("Error: Can't Initialize importer");
		exit(1);
	}

	importStatus = myImporter->Import(pScene); //Import the created scene.

	if (!importStatus) //If the scene can't be opened.
	{
		FBXSDK_printf("Errror: Cant import the created scene.");
		exit(1);
	}

	myImporter->Destroy(); //Destroy the importer because it's no longer required.

	FbxNode* RootNode = pScene->GetRootNode(); //Get the root node, which is a "handle for the FBX contents.

	if (RootNode)
	{
		for (int i = 0; i < RootNode->GetChildCount(); i++)
		{
			FbxNode* ChildNode = RootNode->GetChild(i);

			if (ChildNode->GetNodeAttribute() == NULL) 
				continue;

			FbxNodeAttribute::EType AttributeType = ChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh) //Make sure that only meshes are processed.
				continue;

			myMesh = (FbxMesh*)ChildNode->GetNodeAttribute();
		}
	}
	return myMesh;
}

void ImportVertices(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector)
{
	FbxVector4* vertices = pMesh->GetControlPoints();

	for (int j = 0; j < pMesh->GetPolygonCount(); j++)
	{
		int numberVertices = pMesh->GetPolygonSize(j);
		
		assert(numberVertices == 3);

		for (int i = 0; i < numberVertices; i++)
		{
			int ControlPointIndices = pMesh->GetPolygonVertex(j, i);

			FBXData data;

			data.pos[0] = (float)vertices[ControlPointIndices].mData[0];
			data.pos[1] = (float)vertices[ControlPointIndices].mData[1];
			data.pos[2] = (float)vertices[ControlPointIndices].mData[2];

			outVertexVector->push_back(data);
		}
	}
}

void ImportNormals(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector)
{
	FbxGeometryElementNormal* normalElement = pMesh->GetElementNormal(); //Get the normal element of the mesh.

	if (normalElement)
	{
		if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) //Obtain normal of each vertex.
		{
			//Obtain the normals of each vertex, because the mapping mode of the normal element is by control point.
			for (int vertexIndex = 0; vertexIndex < pMesh->GetControlPointsCount(); vertexIndex++)
			{
				int normalIndex = 0;

				//If reference mode is direct, it means that the normal index is the same as a vertex index.
				if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
				{
					normalIndex = vertexIndex;
				}

				//If the reference mode is Index-to-Direct, it means that the normals are obtained by the Index-to-Direct.
				if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					normalIndex = normalElement->GetIndexArray().GetAt(normalIndex);
				}

				FBXData data;

				FbxVector4 normals = normalElement->GetDirectArray().GetAt(normalIndex); //Normals of each vertex is obtained.

				data.nor[0] = normals.mData[0];
				data.nor[1] = normals.mData[1];
				data.nor[2] = normals.mData[2];

				outVertexVector->push_back(data);
			}

		}

		else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) //Get the normals by obtaining polygon-vertex.
		{
			int indexPolygonVertex = 0;

			//Obtain normals of each polygon, because the mapping mode of normal element is by Polygon-Vertex.
			for (int polygonIndex = 0; polygonIndex < pMesh->GetPolygonCount(); polygonIndex++)
			{
				int polygonSize = pMesh->GetPolygonSize(polygonIndex); //Get the polygon size, to know how many vertices in current polygon.

				for (int i = 0; i < polygonSize; i++) //Obtain each vertex of the current polygon.
				{
					int normalIndex = 0;

					//Reference mode is direct because the normal index is same as indexPolygonVertex.
					if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
					{
						normalIndex = indexPolygonVertex;
					}
					//Reference mose is index-to-direct, which means getting normals by index-to-direct.
					if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					{
						normalIndex = normalElement->GetIndexArray().GetAt(indexPolygonVertex);
					}

					FBXData data;

					FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex); //Obtain normals of each polygon-vertex

					data.nor[0] = normal.mData[0];
					data.nor[1] = normal.mData[1];
					data.nor[2] = normal.mData[2];

					outVertexVector->push_back(data);

					indexPolygonVertex++;
				}
			}
		}
	}
}

void ImportUV(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector)
{
	FbxStringList UVSetNameList; //Obtain the UV set names
	pMesh->GetUVSetNames(UVSetNameList); 

	//Loop through all the uv sets 
	for (int UVSetIndex = 0; UVSetIndex < UVSetNameList.GetCount(); UVSetIndex++)
	{
		const char* UVSetName = UVSetNameList.GetStringAt(UVSetIndex); //Obtain UVSetIndex UV-set
		const FbxGeometryElementUV* UVElement = pMesh->GetElementUV(UVSetName);

		if (!UVElement)
			continue;

		//Only support mapping modes "eByPolygonVertex" and "eByControlPoint".
		if (UVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			UVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return; 

		//Index array, which holds the index referenced to UV data
		bool useIndex = UVElement->GetReferenceMode() != FbxGeometryElement::eIndexToDirect;
		int indexCount = (useIndex) ? UVElement->GetIndexArray().GetCount() : 0;

		//Loop through the data by polygon
		const int polyCount = pMesh->GetPolygonCount();

		//Obtain UV:s with Mapping Mode "eByControlPoint".
		if (UVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
			{
				//Create the max index array, which is required to pass into MakePoly.
				const int polySize = pMesh->GetPolygonSize(polyIndex);

				for (int vertexIndex = 0; vertexIndex < polySize; ++vertexIndex)
				{
					FbxVector2 UVs;

					int polyVertexIndex = pMesh->GetPolygonVertex(polyIndex, vertexIndex); //Obtain index of the current vertex in a control points array.

					int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyIndex) : polyVertexIndex; // UV Index is depending on reference mode.

					UVs = UVElement->GetDirectArray().GetAt(UVIndex);

					FBXData data;

					data.uv[0] = UVs.mData[0];
					data.uv[1] = UVs.mData[1];

					outVertexVector->push_back(data);
				}

			}
		}

		//Obtain UV:s with Mapping Mode "eByPolygonVertex".
		else if (UVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int polyIndexCount = 0;
			for (int polyIndex = 0; polyIndex < polyIndexCount; ++polyIndex)
			{
				//Create the max index array that is required to pass into MakePoly
				const int polySize = pMesh->GetPolygonSize(polyIndex);

				for (int vertexIndex = 0; vertexIndex < polySize; ++vertexIndex)
				{
					if (polyIndexCount < indexCount)
					{
						FbxVector2 UVs;

						int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyIndexCount) : polyIndexCount; //UV Index depends on reference mode.

						UVs = UVElement->GetDirectArray().GetAt(UVIndex);

						FBXData data;

						data.uv[0] = UVs.mData[0];
						data.uv[1] = UVs.mData[1];

						//outVertexVector->push_back(data);

						polyIndexCount++;
					}
				}
			}
		}
	}
}

void CreateTriangleData()
{
	std::vector<FBXData> aVector;

	FbxManager* myManager = nullptr;	//Initialize both the manager and scene as nullptrs.
	FbxScene* myScene = nullptr;
	
	InitiSdkObjects(myManager, myScene);	//Initialize all SDK objects for FBX import. 

	FbxMesh* aMesh = LoadScene(myManager, myScene);		//Import the scene and also return the mesh from the FBX file.

	ImportVertices(aMesh, &aVector);	//Import vertices from FBX. 

	ImportNormals(aMesh, &aVector);		//Import normals from FBX. 

	ImportUV(aMesh, &aVector);			//Import UV:s from FBX.

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = aVector.size() * sizeof(FBXData);

	vertexVector = aVector.size();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = aVector.data();
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);
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
	float clearColor[] = { 0, 0, 0, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);

	gDeviceContext->ClearDepthStencilView(gDepthview, D3D11_CLEAR_DEPTH, 1.0f, 0);

	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
	//gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);

	UINT32 vertexSize = sizeof(FBXData);
	UINT32 offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->IASetInputLayout(gVertexLayout);

	UpdateConstantBuffer();

	gDeviceContext->Draw(vertexVector, 0);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster

	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. Sätt viewport

		CreateShaders(); //4. Skapa vertex- och pixel-shaders

		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		CreateConstantBuffer(); //Calls the CreateConstantBuffer function
		
		CreateTexture();

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

				gSwapChain->Present(0, 0); //9. Växla front- och back-buffer
			}
		}

		gVertexBuffer->Release();
		gConstantBuffer->Release(); //Prevents Memory Leaks
		gDepthStencilBuffer->Release(); //Prevents Memory Leaks

		gDepthview->Release(); //Prevents Memory Leaks
							   /*gTextureView->Release();*/ //Prevents Memory Leaks
															/*gTexture->Release();*/ //Prevents Memory Leaks

		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		//gGeometryShader->Release(); //Prevents Memory Leaks

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
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
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

