#include "Frustum.h"
#include "Linker.h" 
#include "Camera.h"
#include "Engine.h"

Frustum::Frustum()
{

}

Frustum::~Frustum()
{

}

std::vector<XMFLOAT4> Frustum::getFrustumPlanes()
{
	std::vector<XMFLOAT4>tempFrustumPlane(6);
	//get plane from the viewProjection matrix
	
	Matrix viewProj;
	viewProj = cameraPtr->camView * enginePtr->projection;

	//left frustum plane
	tempFrustumPlane[0].x = viewProj._14 + viewProj._11;
	tempFrustumPlane[0].y = viewProj._24 + viewProj._21;
	tempFrustumPlane[0].z = viewProj._34 + viewProj._31;
	tempFrustumPlane[0].w = viewProj._44 + viewProj._41;

	//right frustum plane
	tempFrustumPlane[1].x = viewProj._14 - viewProj._11;
	tempFrustumPlane[1].y = viewProj._24 - viewProj._21;
	tempFrustumPlane[1].z = viewProj._34 - viewProj._31;
	tempFrustumPlane[1].w = viewProj._44 - viewProj._41;

	//top frustum plane
	tempFrustumPlane[2].x = viewProj._14 - viewProj._12;
	tempFrustumPlane[2].y = viewProj._24 - viewProj._22;
	tempFrustumPlane[2].z = viewProj._34 - viewProj._32;
	tempFrustumPlane[2].w = viewProj._44 - viewProj._42;

	//bottum frustum plane
	tempFrustumPlane[3].x = viewProj._14 + viewProj._12;
	tempFrustumPlane[3].y = viewProj._24 + viewProj._22;
	tempFrustumPlane[3].z = viewProj._34 + viewProj._32;
	tempFrustumPlane[3].w = viewProj._44 + viewProj._42;

	//near frustum plane
	tempFrustumPlane[4].x = viewProj._13;
	tempFrustumPlane[4].y = viewProj._23;
	tempFrustumPlane[4].z = viewProj._33;
	tempFrustumPlane[4].w = viewProj._43;

	//far frustum plane
	tempFrustumPlane[5].x = viewProj._14 - viewProj._13;
	tempFrustumPlane[5].y = viewProj._24 - viewProj._23;
	tempFrustumPlane[5].z = viewProj._34 - viewProj._33;
	tempFrustumPlane[5].w = viewProj._44 - viewProj._43;

	//normalize plane normals (A, B, C (xyz))
	//Math: length^2 = A^2 + B^2 + C^2
	//A = A/length, B = B/length, C = C/length, D = D/lenght
	for (int i = 0; i < 6; i++)
	{
		float length = sqrt((tempFrustumPlane[i].x * tempFrustumPlane[i].x) + (tempFrustumPlane[i].y * tempFrustumPlane[i].y) + (tempFrustumPlane[i].z * tempFrustumPlane[i].z));
		tempFrustumPlane[i].x /= length;
		tempFrustumPlane[i].y /= length;
		tempFrustumPlane[i].z /= length;
		tempFrustumPlane[i].w /= length;
	}

	return tempFrustumPlane;
}
