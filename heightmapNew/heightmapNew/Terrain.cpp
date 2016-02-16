#include "Terrain.h"
#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace std;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

Terrain::Terrain()
{

}

Terrain::~Terrain()
{

}

//function that loads a bmp image and stores hm info in the HeightMapInfo structure
bool Terrain::HeightMapLoad(char* filename, HeightMapInfo &heightMInfo)
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
bool Terrain::InitScene()
{
	//the bitmaps filename i being passed to the hmInfo object
	//so it can loaded with the info of the heightmap
	HeightMapInfo hmInfo;
	HeightMapLoad("heightmapImage.bmp", hmInfo);

	//width and length(cols, rows) of the grid(vertices)
	int cols = hmInfo.terrainWidth;
	int rows = hmInfo.terrainHeight;

	//Create the grid 
	numVertices = rows * cols; //to get nr of quads
	numFaces = (rows - 1) * (cols - 1) * 2; //mult with 2 to get the nr of triangles 
											//since it's two triangles in each quad
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