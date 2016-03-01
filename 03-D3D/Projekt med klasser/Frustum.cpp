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

void Frustum::getFrustumPlanes()
{
	
	Matrix viewProj;
	viewProj = cameraPtr->camView * enginePtr->projection;

	//left frustum plane
	frustumPlanes[0].x = viewProj._14 + viewProj._11;
	frustumPlanes[0].y = viewProj._24 + viewProj._21;
	frustumPlanes[0].z = viewProj._34 + viewProj._31;
	frustumPlanes[0].w = viewProj._44 + viewProj._41;

	//right frustum plane
	frustumPlanes[1].x = viewProj._14 - viewProj._11;
	frustumPlanes[1].y = viewProj._24 - viewProj._21;
	frustumPlanes[1].z = viewProj._34 - viewProj._31;
	frustumPlanes[1].w = viewProj._44 - viewProj._41;

	//top frustum plane
	frustumPlanes[2].x = viewProj._14 - viewProj._12;
	frustumPlanes[2].y = viewProj._24 - viewProj._22;
	frustumPlanes[2].z = viewProj._34 - viewProj._32;
	frustumPlanes[2].w = viewProj._44 - viewProj._42;

	//bottum frustum plane
	frustumPlanes[3].x = viewProj._14 + viewProj._12;
	frustumPlanes[3].y = viewProj._24 + viewProj._22;
	frustumPlanes[3].z = viewProj._34 + viewProj._32;
	frustumPlanes[3].w = viewProj._44 + viewProj._42;

	//near frustum plane
	frustumPlanes[4].x = viewProj._13;
	frustumPlanes[4].y = viewProj._23;
	frustumPlanes[4].z = viewProj._33;
	frustumPlanes[4].w = viewProj._43;

	//far frustum plane
	frustumPlanes[5].x = viewProj._14 - viewProj._13;
	frustumPlanes[5].y = viewProj._24 - viewProj._23;
	frustumPlanes[5].z = viewProj._34 - viewProj._33;
	frustumPlanes[5].w = viewProj._44 - viewProj._43;

	//normalize plane normals (A, B, C (xyz))
	//Math: length^2 = A^2 + B^2 + C^2
	//A = A/length, B = B/length, C = C/length, D = D/lenght
	for (int i = 0; i < 6; i++)
	{
		float length = sqrt((frustumPlanes[i].x * frustumPlanes[i].x) + (frustumPlanes[i].y * frustumPlanes[i].y) + (frustumPlanes[i].z * frustumPlanes[i].z));
		frustumPlanes[i].x /= length;
		frustumPlanes[i].y /= length;
		frustumPlanes[i].z /= length;
		frustumPlanes[i].w /= length;
	}

	return;
}

bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
{
	for (int i = 0; i < 6; i++)
	{
		if (XMPlaneDotCoord(&frustumPlanes[i], XMLoadFloat((xCenter + radius), (yCenter - radius), (zCenter - radius))) >= 0.0f;
		{
			continue;
		}
	}

	return false;
}