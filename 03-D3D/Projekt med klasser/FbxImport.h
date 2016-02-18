#ifndef FBXIMPORT_H
#define FBXIMPORT_H

#include "Linker.h"
#include "Engine.h"
#include "System.h"
class System;

struct FBXData
{
	float pos[3];
	float nor[3];
	float uv[2];
};

class FBX
{

private:
	
	FbxManager* pManager;
	FbxScene* pScene;
	FbxNode* RootNode;

public:

	FBX();
	~FBX();

	struct MaterialBuffer
	{
		XMFLOAT3 ambient;
		float transparency;

		XMFLOAT3 diffuse;
		float shininess;

		XMFLOAT3 specular;
		float reflection;

		bool textureBool = false;
		float padding[3];

		Vector4 camPos;
	};

	MaterialBuffer test;

	System* systemPtr;
	
	std::vector<FBXData> outVertexVector;

	ID3D11Texture2D *gTexture = NULL;
	ID3D11SamplerState* sampAni = nullptr;

	ID3D11Buffer* gMaterialBuffer;

	ID3D11ShaderResourceView* gTextureView = nullptr;

	void InitiSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	FbxMesh* LoadScene(FbxManager* pManager, FbxScene* pScene);

	void ImportVertices(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector);
	void ImportNormals(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector);
	void ImportUV(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector);
	void ImportMaterial(FbxMesh* pMesh);
	void ImportTexture(FbxMesh* pMesh);
	void FlipOrder();

	void CreateMaterialBuffer();
	
	void UpdateMaterialBuffer();

	void InitializeModels();

	void RenderFBX();
};

#endif
