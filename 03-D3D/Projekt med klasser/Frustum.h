#ifndef FRUSTUM_H
#define FRUSTUM_H
#include "Linker.h"

class Frustum
{

public:
	Frustum();
	~Frustum();

	void ConstructFrustum(float, Matrix, Matrix);
	bool CheckPoint(float, float, float);
	bool CheckCube(float, float, float, float);

private:

	XMFLOAT4 planes[6];

};

#endif 

