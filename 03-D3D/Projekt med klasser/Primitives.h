#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "Linker.h"
#include "FbxImport.h"
class Engine;
class Camera;
class Terrain;

class Primitives
{
private:

public:
	Primitives();
	~Primitives();

	ID3D11Buffer* primitiveMatrixBuffer = nullptr;
	ID3D11Buffer* pVertexBuffer = nullptr;
	ID3D11ShaderResourceView* pTextureView = nullptr;
	Engine* enginePtr;
	Camera* cameraPtr;
	Terrain* terrainPtr;

	bool CreatePrimitives();
	void RenderPrimitives();
	void CreatePrimitiveMatrixBuffer();
	void createTextures();
	static const int numberOfPrimitives = 6;

	struct PrimitiveBuffer
	{
		Matrix primitiveWVP;
		Matrix primitiveWorld;
	};

	
};


#endif

