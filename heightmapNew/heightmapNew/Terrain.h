#ifndef TERRAIN_H
#define TERRAIN_H


class Terrain
{

public:
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

private:
	Terrain();
	~Terrain();

};

#endif
