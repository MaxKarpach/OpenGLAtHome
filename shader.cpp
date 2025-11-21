#include "shader.h"
#include "model.h"

void SimpleShader::setModelMatrix(const Matrix& model) {
    model_matrix = model;
}

void SimpleShader::setViewMatrix(const Matrix& view) {
    view_matrix = view;
}

void SimpleShader::setProjMatrix(const Matrix& proj) {
    proj_matrix = proj;
}

void SimpleShader::setViewportMatrix(const Matrix& viewport) {
    viewport_matrix = viewport;
}

void SimpleShader::setModel(Model* m) {
    model = m;
}

void SimpleShader::setLightDir(const Vec3f& light) {
    light_dir = light;
}

Vec4f SimpleShader::vertex(int iface, int nthvert) {
    varying_uv.set_col(nthvert, model->uv(iface, nthvert));
    varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
    
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); 
    
    gl_Vertex = model_matrix * gl_Vertex;
    gl_Vertex = view_matrix * gl_Vertex;
    gl_Vertex = proj_matrix * gl_Vertex; 
    gl_Vertex = viewport_matrix * gl_Vertex; 
    
    return gl_Vertex;
}

bool SimpleShader::fragment(Vec3f bar, TGAColor& color) {
    float intensity = varying_intensity * bar;   
    Vec2f uv = varying_uv * bar;
    color = model->diffuse(uv) * intensity; 
    return false;                              
}