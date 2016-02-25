#include "Primitives.h"
#include "Linker.h"
#include "Camera.h"
#include "Engine.h"
#include "Terrain.h"

Primitives::Primitives()
{

}

Primitives::~Primitives()
{
	primitiveMatrixBuffer->Release();
	pVertexBuffer->Release();
}

bool Primitives::CreatePrimitives()
{
	FBXData vertices[] =
	{
		{
			{ 0.0, 0.5, 0.0 },
			{ 0.0, 1.0, 0.0 },
			{ 0.0, 0.0 },
		},
		{
			{ 0.4, -0.5, 0.0 },
			{ 0.0, 1.0, 0.0 },
			{ 1.0, 0.0 },
		},
		{
			{ 0.0, -0.5, 0.0 },
			{ 0.0, 1.0, 0.0 },
			{ 0.0, 1.0 }
		},
	};

	D3D11_BUFFER_DESC VertexBufferDesc;
	memset(&VertexBufferDesc, 0, sizeof(VertexBufferDesc));
	VertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexBufferDesc.ByteWidth = sizeof(FBXData) * 3;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA VertexBufferData;
	memset(&VertexBufferData, 0, sizeof(VertexBufferData));
	VertexBufferData.pSysMem = vertices;

	enginePtr->gDevice->CreateBuffer(&VertexBufferDesc, &VertexBufferData, &pVertexBuffer);

	return true;
}

void Primitives::CreatePrimitiveMatrixBuffer()
{
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(PrimitiveBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	enginePtr->gDevice->CreateBuffer(&desc, NULL, &primitiveMatrixBuffer);
}

void Primitives::UpdatePMatrixBuffer()
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedCB;
	PrimitiveBuffer* dataPtr;

	//Matrix world[5];
	Matrix projection;
	Matrix worldViewProjection;


	Matrix * world;
	world = new Matrix[5];
	
	/*world[0] = XMMatrixTranslation(0.0, 0.0, 0.0);
	world[1] = XMMatrixTranslation(0.5, 0.0, 0.0);
	world[2] = XMMatrixTranslation(1.0, 0.0, 0.0);
	world[3] = XMMatrixTranslation(-0.5, 0.0, 0.0);
	world[4] = XMMatrixTranslation(-1.0, 0.0, 0.0);
	world[5] = XMMatrixTranslation(1.5, 0.0, 0.0);*/

	projection = XMMatrixPerspectiveFovLH(float(3.1415 * 0.45), float(1280.0 / 960.0), float(0.5), float(20));

	float x = 0.5;
	for (int i = 0; i < 6; i++)
	{

		world[i] = XMMatrixTranslation(x, 0.0, 0.0);
		x++;
		worldViewProjection = world[i] * cameraPtr->camView * projection;
		worldViewProjection = worldViewProjection.Transpose();

		world[i] = world[i].Transpose();

		result = enginePtr->gDeviceContext->Map(primitiveMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB);
		if (FAILED(result))
		{
			return;
		}

		dataPtr = (PrimitiveBuffer*)mappedCB.pData;
		dataPtr->primitiveWVP = worldViewProjection;
		dataPtr->primitiveWorld = world[i];
	}
	enginePtr->gDeviceContext->Unmap(primitiveMatrixBuffer, 0);

	enginePtr->gDeviceContext->VSSetConstantBuffers(0, 1, &primitiveMatrixBuffer);

}

void Primitives::RenderPrimitives()
{
	UINT32 offset = 0;
	UINT32 vertexPrimitiveSize = sizeof(FBXData);
	
	//enginePtr->gDeviceContext->IASetIndexBuffer(gIndexBuffer, DXGI_FORMAT_R32_UINT, offset);
	//enginePtr->gDeviceContext->DrawIndexed(numFaces * 3, 0, 0);

	for (int j = 0; j < 6; j++)
	{
		enginePtr->gDeviceContext->PSSetShaderResources(0, 1, &terrainPtr->hTextureView);
		enginePtr->gDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &vertexPrimitiveSize, &offset);
		enginePtr->gDeviceContext->Draw(3, 0);
		
	}

}
