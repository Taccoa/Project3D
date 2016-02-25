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
	/*bool CheckPoint(float, float, float);
	/*bool CheckCube(float, float, float, float);*/
	Camera* cameraPtr;
	Engine* enginePtr;

	std::vector<XMFLOAT4> getFrustumPlanes();

private:

};

#endif 

