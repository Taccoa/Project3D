#include "Frustum.h"
#include "Linker.h" 

Frustum::Frustum()
{

}

Frustum::~Frustum()
{

}

std::vector<XMFLOAT4> Frustum::getFrustumPlanes()
{
	std::vector<XMFLOAT4>tempFrustumPlane(6);
	FrustumMartix m;
	//get plane from the viewProjection matrix

	//left frustum plane
	tempFrustumPlane[0].x = m.viewProj._14 + m.viewProj._11;
	tempFrustumPlane[0].x = m.viewProj._24 + m.viewProj._21;
	tempFrustumPlane[0].x = m.viewProj._34 + m.viewProj._31;
	tempFrustumPlane[0].x = m.viewProj._44 + m.viewProj._41;

	//right frustum plane
	tempFrustumPlane[1].x = m.viewProj._14 - m.viewProj._11;
	tempFrustumPlane[1].y = m.viewProj._24 - m.viewProj._21;
	tempFrustumPlane[1].z = m.viewProj._34 - m.viewProj._31;
	tempFrustumPlane[1].w = m.viewProj._44 - m.viewProj._41;

	//top frustum plane
	tempFrustumPlane[2].x = m.viewProj._14 - m.viewProj._12;
	tempFrustumPlane[2].y = m.viewProj._24 - m.viewProj._22;
	tempFrustumPlane[2].z = m.viewProj._34 - m.viewProj._32;
	tempFrustumPlane[2].w = m.viewProj._44 - m.viewProj._42;

	//bottum frustum plane
	tempFrustumPlane[3].x = m.viewProj._14 + m.viewProj._12;
	tempFrustumPlane[3].y = m.viewProj._24 + m.viewProj._22;
	tempFrustumPlane[3].z = m.viewProj._34 + m.viewProj._32;
	tempFrustumPlane[3].w = m.viewProj._44 + m.viewProj._42;

	//near frustum plane
	tempFrustumPlane[4].x = m.viewProj._13;
	tempFrustumPlane[4].y = m.viewProj._23;
	tempFrustumPlane[4].z = m.viewProj._33;
	tempFrustumPlane[4].w = m.viewProj._43;

	//far frustum plane
	tempFrustumPlane[5].x = m.viewProj._14 - m.viewProj._13;
	tempFrustumPlane[5].y = m.viewProj._24 - m.viewProj._23;
	tempFrustumPlane[5].z = m.viewProj._34 - m.viewProj._33;
	tempFrustumPlane[5].w = m.viewProj._44 - m.viewProj._43;

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

/*void Frustum::ConstructFrustum(float screenDepth, Matrix projMatrix, Matrix viewMatrix)
{
	float zMin, r;
	Matrix matrix;
	Matrices m;

	zMin = m.projMatrix._43 / m.projMatrix._33;
	r = m.screenDepth / (m.screenDepth - zMin);
	m.projMatrix._33 = r;
	m.projMatrix._43 = -r*zMin;

	Matrix frustumMatrix = matrix * m.viewMatrix * m.projMatrix;

	//near plane frustum
	planes[0].x = matrix._13;
	planes[0].y = matrix._23;
	planes[0].z = matrix._33;
	planes[0].w = matrix._43;
	XMPlaneNormalize(XMLoadFloat4(&planes[0]));

	//far plane frustum
	planes[1].x = matrix._14 - matrix._13;
	planes[1].y = matrix._24 - matrix._23;
	planes[1].z = matrix._34 - matrix._33;
	planes[1].w = matrix._44 - matrix._43;
	XMPlaneNormalize(XMLoadFloat4(&planes[1]));

	//left plane frustum
	planes[2].x = matrix._14 + matrix._11;
	planes[2].y = matrix._24 + matrix._21;
	planes[2].z = matrix._34 + matrix._31;
	planes[2].w = matrix._44 + matrix._41;
	XMPlaneNormalize(XMLoadFloat4(&planes[2]));

	//right plane frustum
	planes[3].x = matrix._14 - matrix._11;
	planes[3].y = matrix._24 - matrix._21;
	planes[3].z = matrix._34 - matrix._31;
	planes[3].w = matrix._44 - matrix._41;
	XMPlaneNormalize(XMLoadFloat4(&planes[3]));

	//top plane frustum
	planes[4].x = matrix._14 - matrix._12;
	planes[4].y = matrix._24 - matrix._22;
	planes[4].z = matrix._34 - matrix._32;
	planes[4].w = matrix._44 - matrix._42;
	XMPlaneNormalize(XMLoadFloat4(&planes[4]));

	//bottum plane frustum
	planes[5].x = matrix._14 + matrix._12;
	planes[5].y = matrix._24 + matrix._22;
	planes[5].z = matrix._34 + matrix._32;
	planes[5].w = matrix._44 + matrix._42;
	XMPlaneNormalize(XMLoadFloat4(&planes[5]));

	return;
}*/



//Checks if a single pint is inside the view frustum
//if true the point is inside all six of the planes
/*bool Frustum::CheckPoint(float x, float y, float z)
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
}*/



