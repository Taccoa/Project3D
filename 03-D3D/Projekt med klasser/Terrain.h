#ifndef TERRAIN_H
#define TERRAIN_H

#include "Linker.h"
#include "Engine.h"
#include "Camera.h"
#include "FBXImport.h"

class Engine;
class Camera;

class Terrain
{

public:

	Terrain();
	~Terrain();

	Camera* cameraPtr;
	
	struct HeightMapInfo
	{
		int terrainHeight;
		int terrainWidth;
		XMFLOAT3 *heightMap; //array to store terrain's vertex positions
	};

	struct TMatrixBuffer
	{
		Matrix terrainWVP;
		Matrix terrainWorld;
	};

	int numFaces = 0;
	int numVertices = 0;

	bool HeightMapLoad(char* filename, HeightMapInfo &heightMInfo);
	bool InitScene();

	ID3D11Buffer* terrainMatrixBuffer = nullptr;
	ID3D11Buffer* hVertexBuffer = nullptr;
	ID3D11ShaderResourceView* hTextureView;
	ID3D11Texture2D *hTexture = NULL;
	ID3D11Buffer* gIndexBuffer = nullptr;

	void CreateHeightTexture();

	Engine* enginePtr;

	void RenderTerrain();

	void CreateTerrainMatrixBuffer();
	void UpdateTerrainMatrixBuffer();

private:

};

#endif
