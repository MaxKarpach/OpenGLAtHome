#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "geometry.h"


class Camera 
{
private:
	Vec3f eye_;
	Vec3f center_;
	Vec3f up_;
public:
	Camera(Vec3f eye, Vec3f center, Vec3f up) : eye_(eye), center_(center), up_(up){}
	Matrix viewportMatrix(int x, int y, int w, int h, int d);
	Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);
	Matrix proj(Vec3f eye, Vec3f center);
};


#endif //__CAMERA_H__