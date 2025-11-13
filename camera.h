#ifndef CAMERA_H
#define CAMERA_H

#include "geometry.h"

class Camera {
public:
    Vec3f eye;
    Vec3f center;
    Vec3f up;

    Camera(Vec3f eye, Vec3f center, Vec3f up);
    Matrix lookat();
};

#endif