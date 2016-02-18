#include "FbxImport.h"

FBX::FBX()
{
}

FBX::~FBX()
{
	gMaterialBuffer->Release();
	pManager->Destroy();
}

void FBX::InitiSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
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

FbxMesh* FBX::LoadScene(FbxManager* pManager, FbxScene* pScene)
{
	FbxImporter* myImporter = FbxImporter::Create(pManager, "My importer"); //Creates the importer to use with SDK.

	FbxMesh* myMesh = nullptr;

	bool importStatus = myImporter->Initialize("D:/test.FBX", -1, pManager->GetIOSettings()); //Initialize the importer with a filename

	/*Beware: Use only front slash for the filepath of the model that is to be imported. Otherwise importStatus would return false!*/

	if (!importStatus) //If the importer can't be initialized.
	{
		FbxString error = myImporter->GetStatus().GetErrorString();
		FBXSDK_printf("Error: Can't Initialize importer");
		exit(1);
	}

	importStatus = myImporter->Import(pScene); //Import the created scene.

	if (!importStatus) //If the scene can't be opened.
	{
		FBXSDK_printf("Error: Cant import the created scene.");
		exit(1);
	}

	myImporter->Destroy(); //Destroy the importer because it's no longer required.

	RootNode = pScene->GetRootNode(); //Get the root node, which is a "handle for the FBX contents.

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

void FBX::ImportVertices(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector)
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
			data.pos[2] = -(float)vertices[ControlPointIndices].mData[2];

			outVertexVector->push_back(data);
		}
	}
}

void FBX::ImportNormals(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector)
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

				FbxVector4 normals = normalElement->GetDirectArray().GetAt(normalIndex); //Normals of each vertex is obtained.

				outVertexVector->at(vertexIndex).nor[0] = normals.mData[0];
				outVertexVector->at(vertexIndex).nor[1] = normals.mData[1];
				outVertexVector->at(vertexIndex).nor[2] = -normals.mData[2];
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

					FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex); //Obtain normals of each polygon-vertex

					outVertexVector->at(indexPolygonVertex).nor[0] = normal.mData[0];
					outVertexVector->at(indexPolygonVertex).nor[1] = normal.mData[1];
					outVertexVector->at(indexPolygonVertex).nor[2] = -normal.mData[2];

					indexPolygonVertex++;
				}
			}
		}
	}
}

void FBX::ImportUV(FbxMesh* pMesh, std::vector<FBXData>* outVertexVector)
{
	FbxStringList UVSetNameList;
	pMesh->GetUVSetNames(UVSetNameList);

	for (int setIndex = 0; setIndex < UVSetNameList.GetCount(); setIndex++)
	{
		const char* UVSetName = UVSetNameList.GetStringAt(setIndex);
		const FbxGeometryElementUV* UVElement = pMesh->GetElementUV(UVSetName);

		if (!UVElement)
			continue;

		if (UVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			UVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return;

		const bool useIndex = UVElement->GetReferenceMode() != FbxGeometryElement::eDirect &&
			UVElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect;

		const int indexCount = (useIndex) ? UVElement->GetIndexArray().GetCount() : 0;

		const int polyCount = pMesh->GetPolygonCount();

		if (UVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
			{
				const int polySize = pMesh->GetPolygonSize(polyIndex);

				for (int vertexIndex = 0; vertexIndex < polySize; ++vertexIndex)
				{
					FbxVector2 UVs;

					int polyVertexIndex = pMesh->GetPolygonVertex(polyIndex, vertexIndex);

					int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyVertexIndex) : polyVertexIndex;

					UVs = UVElement->GetDirectArray().GetAt(UVIndex);

					outVertexVector->at(vertexIndex).uv[0] = UVs.mData[0];
					outVertexVector->at(vertexIndex).uv[1] = 1 - UVs.mData[1];
				}
			}
		}
		else if (UVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int polyIndexCount = 0;
			for (int polyIndex = 0; polyIndex < polyCount; ++polyIndex)
			{
				const int polySize = pMesh->GetPolygonSize(polyIndex);
				for (int vertexIndex = 0; vertexIndex < polySize; ++vertexIndex)
				{
					FbxVector2 UVs;

					int UVIndex = useIndex ? UVElement->GetIndexArray().GetAt(polyIndexCount) : polyIndexCount;

					UVs = UVElement->GetDirectArray().GetAt(UVIndex);

					outVertexVector->at(polyIndexCount).uv[0] = UVs.mData[0];
					outVertexVector->at(polyIndexCount).uv[1] = 1 - UVs.mData[1];

					polyIndexCount++;
				}
			}
		}
	}
}

void FBX::ImportMaterial(FbxMesh* pMesh)
{
	int materialCount = 0;

	if (pMesh)
	{
		materialCount = pMesh->GetNode()->GetMaterialCount();
	}

	if (materialCount > 0)
	{
		FbxPropertyT<FbxDouble3> ambient;
		FbxPropertyT<FbxDouble3> diffuse;
		FbxPropertyT<FbxDouble3> specular;

		FbxPropertyT<FbxDouble> transparency;
		FbxPropertyT<FbxDouble> shininess;
		FbxPropertyT<FbxDouble> reflection;


		for (int materialIndex = 0; materialIndex < materialCount; materialIndex++)
		{
			FbxSurfaceMaterial* material = pMesh->GetNode()->GetMaterial(materialIndex);

			FbxString materialName = material->GetName(); //To see what the name of the material is. 

			if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				ambient = ((FbxSurfacePhong*)material)->Ambient;
				diffuse = ((FbxSurfacePhong*)material)->Diffuse;
				specular = ((FbxSurfacePhong*)material)->Specular;

				transparency = ((FbxSurfacePhong*)material)->TransparencyFactor;
				shininess = ((FbxSurfacePhong*)material)->Shininess;
				reflection = ((FbxSurfacePhong*)material)->ReflectionFactor;

				test.ambient.x = ambient.Get()[0];
				test.ambient.y = ambient.Get()[1];
				test.ambient.z = ambient.Get()[2];

				test.diffuse.x = diffuse.Get()[0];
				test.diffuse.y = diffuse.Get()[1];
				test.diffuse.z = diffuse.Get()[2];

				test.specular.x = specular.Get()[0];
				test.specular.y = specular.Get()[1];
				test.specular.z = specular.Get()[2];

				test.transparency = transparency.Get();
				test.shininess = shininess.Get();
				test.reflection = reflection.Get();
			}

			else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				ambient = ((FbxSurfaceLambert*)material)->Ambient;
				diffuse = ((FbxSurfaceLambert*)material)->Diffuse;
				transparency = ((FbxSurfaceLambert*)material)->TransparencyFactor;

				test.ambient.x = ambient.Get()[0];
				test.ambient.y = ambient.Get()[1];
				test.ambient.z = ambient.Get()[2];

				test.diffuse.x = diffuse.Get()[0];
				test.diffuse.y = diffuse.Get()[1];
				test.diffuse.z = diffuse.Get()[2];

				test.specular.x = 0;
				test.specular.y = 0;
				test.specular.z = 0;

				test.transparency = transparency.Get();
			}

			else
			{
				FBXSDK_printf("Error: Unknown material.\n");
			}
		}
	}
}

void FBX::ImportTexture(FbxMesh* pMesh)
{
	FbxProperty prop;

	if (pMesh->GetNode() == NULL)
		return;

	int nbMat = pMesh->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();

	for (int materialIndex = 0; materialIndex < nbMat; materialIndex++)
	{
		FbxSurfaceMaterial *material = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(materialIndex);

		if (material)
		{
			prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

			if (prop.IsValid())
			{
				int textureCount = prop.GetSrcObjectCount<FbxTexture>();

				for (int j = 0; j < textureCount; ++j)
				{
					FbxLayeredTexture *layerTexture = prop.GetSrcObject<FbxLayeredTexture>(j);

					if (layerTexture)
					{
						FbxLayeredTexture * layeredTexture = prop.GetSrcObject <FbxLayeredTexture>(j);
						int nbTextures = layeredTexture->GetSrcObjectCount<FbxTexture>();

						for (int k = 0; k < nbTextures; ++k)
						{
							FbxTexture* texture = layeredTexture->GetSrcObject<FbxTexture>(k);

							if (texture)
							{
								FbxLayeredTexture::EBlendMode blendMode;

								layeredTexture->GetTextureBlendMode(k, blendMode);
							}

						}
					}

					FbxTexture* texture = prop.GetSrcObject<FbxTexture>(j);

					FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(texture);

					FbxString filetextureName = fileTexture->GetFileName();

					wchar_t* out;
					FbxUTF8ToWC(filetextureName.Buffer(), out, NULL);

					HRESULT hr = CreateWICTextureFromFile(systemPtr->enginePtr->gDevice, systemPtr->enginePtr->gDeviceContext, out, NULL, &gTextureView, 0);

					FbxFree(out);

					test.textureBool = true;
				}
			}
			else
			{
				test.textureBool = false;
			}
		}
	}
}

void FBX::FlipOrder()
{
	/*The winding order is not correct when importing the Position, Normals and UV:s,
	which can be solved by looping through the vector and flip the order in each triangle.*/

	for (unsigned int i = 0; i < outVertexVector.size(); i += 3)
	{
		/*To flip the order of the triangles the first 2 points in the triangles are swapped,
		which in this case mirrors the rendered mesh in the future scene.*/
		std::swap(outVertexVector[i], outVertexVector[i + 1]); 
	}
}

void FBX::CreateMaterialBuffer()
{
	D3D11_BUFFER_DESC desc;

	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(MaterialBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	systemPtr->enginePtr->gDevice->CreateBuffer(&desc, NULL, &gMaterialBuffer);
}

void FBX::UpdateMaterialBuffer()
{
	D3D11_MAPPED_SUBRESOURCE subr;

	systemPtr->enginePtr->gDeviceContext->Map(gMaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subr);

	memcpy(subr.pData, &test, sizeof(MaterialBuffer));

	systemPtr->enginePtr->gDeviceContext->Unmap(gMaterialBuffer, 0);

	systemPtr->enginePtr->gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);
}

void FBX::InitializeModels()
{
	InitiSdkObjects(pManager, pScene); //Initialize all SDK objects for FBX import. 

	FbxMesh* pMesh = LoadScene(pManager, pScene); //Import the scene and also return the mesh from the FBX file.

	ImportVertices(pMesh, &outVertexVector); //Import vertices from FBX. 

	ImportNormals(pMesh, &outVertexVector);		//Import normals from FBX. 

	ImportUV(pMesh, &outVertexVector);			//Import UV:s from FBX.

	ImportMaterial(pMesh);

	ImportTexture(pMesh);

	FlipOrder();

	CreateMaterialBuffer();

	UpdateMaterialBuffer();

	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = outVertexVector.size() * sizeof(FBXData);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = outVertexVector.data();
	systemPtr->enginePtr->gDevice->CreateBuffer(&bufferDesc, &data, &systemPtr->enginePtr->gVertexBuffer);
}

void FBX::RenderFBX()
{
	systemPtr->enginePtr->gDeviceContext->PSSetShaderResources(0, 1, &gTextureView);

	UINT32 vertexSize = sizeof(FBXData);
	UINT32 offset = 0;
	systemPtr->enginePtr->gDeviceContext->IASetVertexBuffers(0, 1, &systemPtr->enginePtr->gVertexBuffer, &vertexSize, &offset);

	systemPtr->enginePtr->gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	systemPtr->enginePtr->gDeviceContext->IASetInputLayout(systemPtr->enginePtr->gVertexLayout);

	systemPtr->enginePtr->gDeviceContext->PSSetConstantBuffers(0, 1, &gMaterialBuffer);

	systemPtr->enginePtr->gDeviceContext->Draw(outVertexVector.size(), 0);
}
