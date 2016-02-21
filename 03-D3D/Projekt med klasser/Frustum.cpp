#include "Frustum.h"

Frustum::Frustum()
{

}

Frustum::~Frustum()
{

}

void Frustum::ConstructFrustum(float screenDepth, Matrix projMatrix, Matrix viewMatrix)
{
	float zMin, r;
	Matrix matrix;

	zMin = projMatrix._43 / projMatrix._33;
	r = screenDepth / (screenDepth - zMin);
	projMatrix._33 = r;
	projMatrix._43 = -r*zMin;

	Matrix frustumMatrix = matrix * viewMatrix * projMatrix;

	//near plane frustum
	planes[0].x = matrix._14 + matrix._13;
	planes[0].y = matrix._24 + matrix._23;
	planes[0].z = matrix._34 + matrix._33;
	planes[0].w = matrix._44 + matrix._43;

	XMPlaneNormalize(XMLoadFloat4(&planes[0]));

	//far plane frustum
	planes[1].x = matrix._14 + matrix._13;
	planes[1].y = matrix._24 + matrix._23;
	planes[1].z = matrix._34 + matrix._33;
	planes[1].w = matrix._44 + matrix._43;
	XMPlaneNormalize(XMLoadFloat4(&planes[1]));

	//far plane frustum
	planes[2].x = matrix._14 + matrix._11;
	planes[2].y = matrix._24 + matrix._21;
	planes[2].z = matrix._34 + matrix._31;
	planes[2].w = matrix._44 + matrix._41;
	XMPlaneNormalize(XMLoadFloat4(&planes[2]));


	planes[3].x = matrix._14 + matrix._11;
	planes[3].y = matrix._24 + matrix._21;
	planes[3].z = matrix._34 + matrix._31;
	planes[3].w = matrix._44 + matrix._41;
	XMPlaneNormalize(XMLoadFloat4(&planes[3]));

	planes[4].x = matrix._14 + matrix._12;
	planes[4].y = matrix._24 + matrix._22;
	planes[4].z = matrix._34 + matrix._32;
	planes[4].w = matrix._44 + matrix._42;
	XMPlaneNormalize(XMLoadFloat4(&planes[4]));

	planes[5].x = matrix._14 + matrix._12;
	planes[5].y = matrix._24 + matrix._22;
	planes[5].z = matrix._34 + matrix._32;
	planes[5].w = matrix._44 + matrix._42;
	XMPlaneNormalize(XMLoadFloat4(&planes[5]));

	return;
}

//Checks if a single pint is inside the view frustum
//if true the point is inside all six of the planes
bool Frustum::CheckPoint(float x, float y, float z)
{
	XMFLOAT3 coords[3];
	
	for (int i = 0; i < 6; i++)
	{
		if (planes[i].x * coords[0] + planes[i].y * coords[1] + planes[i].z * coords[2] + planes[i].w <= 0.0f)
		{
			return false;
		}
	}
	return true;
}



