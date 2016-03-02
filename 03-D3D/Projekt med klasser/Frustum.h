#ifndef FRUSTUM_H
#define FRUSTUM_H
#include "Linker.h"
class Engine;
class Camera;

class Frustum
{

public:
	Frustum();
	~Frustum();

	//void ConstructFrustum(float screenDepth, Matrix projMatrix,	Matrix viewMatrix);
	/*bool CheckPoint(float, float, float);*/
	
	Camera* cameraPtr;
	Engine* enginePtr;

	XMFLOAT4 frustumPlanes[6];

	void getFrustumPlanes();
	//bool CheckCube(XMFLOAT4 coords);

private:

};

#endif 

