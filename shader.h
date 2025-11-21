#ifndef __SHADER_H__
#define __SHADER_H__

#include "tgaimage.h"
#include "geometry.h"

class Model;

class IShader {
public:
    virtual ~IShader() {}
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

class SimpleShader : public IShader {
public:
    Model* model;
    Matrix model_matrix;
    Matrix view_matrix;
    Matrix proj_matrix;
    Matrix viewport_matrix;
    Vec3f light_dir;
    
    Vec3f varying_intensity; 
    mat<2, 3, float> varying_uv;

    void setModelMatrix(const Matrix& model);
    void setViewMatrix(const Matrix& view);
    void setProjMatrix(const Matrix& proj);
    void setViewportMatrix(const Matrix& viewport);
    void setModel(Model* m);
    void setLightDir(const Vec3f& light);
    
    virtual Vec4f vertex(int iface, int nthvert);
    virtual bool fragment(Vec3f bar, TGAColor& color);
};

#endif //__SHADER_H__