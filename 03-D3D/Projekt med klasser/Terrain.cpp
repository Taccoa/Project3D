#include "Terrain.h"

#include "Linker.h"
#include "bth_image.h"
#include "Camera.h"
#include "Engine.h"
#include "System.h"

Terrain::Terrain()
{

}

Terrain::~Terrain()
{
	hVertexBuffer->Release();
	hTextureView->Release();
	hTexture->Release();
	hIndexBuffer->Release();
	//----------------------------
	terrainMatrixBuffer->Release();
	terrainMaterialBuffer->Release();
	//----------------------------
}

//function that loads a bmp image and stores hm info in the HeightMapInfo structure
bool Terrain::HeightMapLoad(char* filename, HeightMapInfo &heightMInfo)
{
	FILE *filePointer;					//point to the current position in the file
	BITMAPFILEHEADER bitmapFileHeader;	//structure that contains info about type, size, layout of the file
	BITMAPINFOHEADER bitmapInfoHeader;	//contains info about the image inside the file
	int index;				//index = keeps track of current place in the grid 
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
bool Terrain::InitScene()
{
	//--------------------------------------------------------
	material.ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);
	material.transparency = float(0.0f);
	material.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
	material.shininess = float(1.0f);
	material.specular = XMFLOAT3(1.0f, 1.0f, 1.0f);
	material.reflection = float(0.2f);
	material.textureBool = true;
	//--------------------------------------------------------

	//the bitmaps filename i being passed to the hmInfo object
	//so it can loaded with the info of the heightmap
	HeightMapInfo hmInfo;
	HeightMapLoad("heightMap.bmp", hmInfo);

	//width and length(cols, rows) of the grid(vertices)
	unsigned int cols = hmInfo.terrainWidth;
	unsigned int rows = hmInfo.terrainHeight;

	//Create the grid 
	numVertices = rows * cols; //to get nr of quads
	numFaces = (rows - 1) * (cols - 1) * 2; //mult with 2 to get the nr of triangles 
											//since it's two triangles in each quad
											//vector to hold all the vertices
	std::vector<TerrainData> v(numVertices);

	//loop throgh each col and row of the grid
	for (DWORD i = 0; i < rows; i++)
	{
		for (DWORD j = 0; j < cols; j++)
		{
			//define the position of each vertex by using hmInfos info
			v[i*cols + j].pos[0] = hmInfo.heightMap[i*cols + j].x;
			v[i*cols + j].pos[1] = hmInfo.heightMap[i*cols + j].y;
			v[i*cols + j].pos[2] = hmInfo.heightMap[i*cols + j].z;
			//initializing the normal so it points directly up, faces of the terrain will be lit equally
			//if you have a normal defined in the hmap file, set it here the same way as the position(saves runtime) 
			v[i*cols + j].nor[0] = XMFLOAT3(0.0f, 1.0f, 0.0f).x;
			v[i*cols + j].nor[1] = XMFLOAT3(0.0f, 1.0f, 0.0f).y;
			v[i*cols + j].nor[2] = XMFLOAT3(0.0f, 1.0f, 0.0f).z;
			v[i*cols + j].uv[0] = XMFLOAT2(float(i) / rows, float(j) / cols).x;
			v[i*cols + j].uv[1] = XMFLOAT2(float(i) / rows, float(j) / cols).y;
		}
	}

	delete[] hmInfo.heightMap;

	std::vector<DWORD> indices(numFaces * 3);

	int k = 0;
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

	D3D11_BUFFER_DESC VertexBufferDesc;
	memset(&VertexBufferDesc, 0, sizeof(VertexBufferDesc));
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.ByteWidth = sizeof(TerrainData) * numVertices;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA VertexBufferData;
	memset(&VertexBufferData, 0, sizeof(VertexBufferData));
	VertexBufferData.pSysMem = &v[0];

	enginePtr->gDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferData, &hVertexBuffer);

	D3D11_BUFFER_DESC IndexBufferDesc;
	memset(&IndexBufferDesc, 0, sizeof(IndexBufferDesc));
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.ByteWidth = sizeof(DWORD) * numFaces * 3;
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA IndexBufferData;
	memset(&IndexBufferData, 0, sizeof(IndexBufferData));
	IndexBufferData.pSysMem = &indices[0];

	enginePtr->gDevice->CreateBuffer(&IndexBufferDesc, &IndexBufferData, &hIndexBuffer);

	return true;
}

void Terrain::CreateHeightTexture()
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
	enginePtr->gDevice->CreateTexture2D(&bthTexDesc, &data, &hTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc, sizeof(resViewDesc));
	resViewDesc.Format = bthTexDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = bthTexDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	enginePtr->gDevice->CreateShaderResourceView(hTexture, &resViewDesc, &hTextureView);
}

void Terrain::RenderTerrain()
{
	
	enginePtr->gDeviceContext->PSSetShaderResources(0, 1, &hTextureView);

	UINT32 offset = 0;
	UINT32 vertexTerrainSize = sizeof(TerrainData);
	enginePtr->gDeviceContext->IASetVertexBuffers(0, 1, &hVertexBuffer, &vertexTerrainSize, &offset);
	enginePtr->gDeviceContext->IASetIndexBuffer(hIndexBuffer, DXGI_FORMAT_R32_UINT, offset);
	enginePtr->gDeviceContext->DrawIndexed(numFaces * 3, 0, 0);
	
}

void Terrain::CreateTerrainMatrixBuffer()
{
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(TMatrixBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	enginePtr->gDevice->CreateBuffer(&desc, NULL, &terrainMatrixBuffer);
}

void Terrain::UpdateTerrainMatrixBuffer()
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedCB;
	TMatrixBuffer* dataPtr;

	Matrix world;
	Matrix projection;
	Matrix worldViewProjection;

	world = XMMatrixScaling(0.18, 0.18, 0.18) * XMMatrixTranslation(0.0, -1.5, 0.0);

	projection = XMMatrixPerspectiveFovLH(float(3.1415 * 0.45), float(1280.0 / 960.0), float(0.5), float(20));

	worldViewProjection = world * cameraPtr->camView * projection;
	worldViewProjection = worldViewProjection.Transpose();

	world = world.Transpose();

	result = enginePtr->gDeviceContext->Map(terrainMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB);
	if (FAILED(result))
	{
		return;
	}

	dataPtr = (TMatrixBuffer*)mappedCB.pData;
	dataPtr->terrainWVP = worldViewProjection;
	dataPtr->terrainWorld = world;

	enginePtr->gDeviceContext->Unmap(terrainMatrixBuffer, 0);

	enginePtr->gDeviceContext->VSSetConstantBuffers(0, 1, &terrainMatrixBuffer);

}

//---------------------------------------------------------------------
void Terrain::createTerrainMaterialBuffer()
{

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(FBX::MaterialBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	enginePtr->gDevice->CreateBuffer(&desc, NULL, &terrainMaterialBuffer);
}

void Terrain::updateTerrainMaterialBuffer()
{
	D3D11_MAPPED_SUBRESOURCE tSubr;

	HRESULT hr= enginePtr->gDeviceContext->Map(terrainMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &tSubr);

	memcpy(tSubr.pData, &material, sizeof(FBX::MaterialBuffer));

	enginePtr->gDeviceContext->Unmap(terrainMaterialBuffer, 0);

	enginePtr->gDeviceContext->PSSetConstantBuffers(0, 1, &terrainMaterialBuffer);
	
}
//----------------------------------------------------------------------
