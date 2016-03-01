#ifndef TERRAIN_H
#define TERRAIN_H

#include "Linker.h"
#include "FBXImport.h"

class Engine;
class Camera;

struct TerrainData
{
	float pos[3];
	float nor[3];
	float uv[2];
};

class Terrain
{

public:

	Terrain();
	~Terrain();

	Camera* cameraPtr;
	Engine* enginePtr;

	//-----------------------------
	FBX::MaterialBuffer material;
	//-----------------------------

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
	ID3D11Buffer* terrainMaterialBuffer = nullptr;
	ID3D11Buffer* hVertexBuffer = nullptr;
	ID3D11ShaderResourceView* hTextureView = nullptr;
	ID3D11Texture2D *hTexture = NULL;
	ID3D11Buffer* hIndexBuffer = nullptr;

	void CreateHeightTexture();

	void RenderTerrain();

	void CreateTerrainMatrixBuffer();
	void UpdateTerrainMatrixBuffer();

	//------------------------------------
	void createTerrainMaterialBuffer();
	void updateTerrainMaterialBuffer();
	//------------------------------------

private:

};

#endif
