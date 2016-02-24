#ifndef FRUSTUM_H
#define FRUSTUM_H
#include "Linker.h"

class Frustum
{

public:
	Frustum();
	~Frustum();

	//void ConstructFrustum(float screenDepth, Matrix projMatrix,	Matrix viewMatrix);
	/*bool CheckPoint(float, float, float);
	/*bool CheckCube(float, float, float, float);*/

	std::vector<XMFLOAT4> getFrustumPlanes();

	struct FrustumMartix
	{
		Matrix viewProj;
	};

private:

};

#endif 

