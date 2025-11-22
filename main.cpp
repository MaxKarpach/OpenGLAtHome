#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "shader.h"
#include "camera.h"

Model* model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;
Vec3f light_dir(1, 1, 1);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

std::vector<Vec3f> create_cube_vertices(float size) {
    std::vector<Vec3f> vertices;
    float s = size / 2.0f;
    
    vertices.push_back(Vec3f(-s, -s, -s));
    vertices.push_back(Vec3f(s, -s, -s));
    vertices.push_back(Vec3f(s, s, -s));
    vertices.push_back(Vec3f(-s, s, -s));
    vertices.push_back(Vec3f(-s, -s, s));
    vertices.push_back(Vec3f(s, -s, s));
    vertices.push_back(Vec3f(s, s, s)); 
    vertices.push_back(Vec3f(-s, s, s));
    
    return vertices;
}

std::vector<std::vector<int>> create_cube_faces() {
    std::vector<std::vector<int>> faces;
    
    faces.push_back({0, 1, 2});
    faces.push_back({0, 2, 3});
    faces.push_back({4, 6, 5});
    faces.push_back({4, 7, 6});
    faces.push_back({0, 4, 5});
    faces.push_back({0, 5, 1});
    faces.push_back({3, 2, 6});
    faces.push_back({3, 6, 7});
    faces.push_back({0, 3, 7});
    faces.push_back({0, 7, 4});
    faces.push_back({1, 5, 6});
    faces.push_back({1, 6, 2});
    
    return faces;
}

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
            float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
            if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}

void draw_triangle_with_transparency(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    
    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), 
                                proj<2>(pts[1] / pts[1][3]), 
                                proj<2>(pts[2] / pts[2][3]), 
                                proj<2>(P));
            
            float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
            float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
            int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
            
            if (c.x < 0 || c.y < 0 || c.z < 0) continue;
            
            bool discard = shader.fragment(c, color);
            if (!discard) {
                if (color[3] == 255) {
                    if (zbuffer.get(P.x, P.y)[0] < frag_depth) {
                        zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                        image.set(P.x, P.y, color);
                    }
                } else {
                    float alpha = color[3] / 255.0f;
                    if (alpha > 0.1f) {
                        TGAColor background = image.get(P.x, P.y);
                        TGAColor blended;
                        for (int i = 0; i < 3; i++) {
                            blended[i] = background[i] * (1 - alpha) + color[i] * alpha;
                        }
                        blended[3] = 255;
                        image.set(P.x, P.y, blended);
                    }
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    model = new Model("obj/african_head.obj");
    light_dir.normalize();

    Vec3f eye_position(1, 2, 3);
    
    Camera camera(eye_position, center, up);
    
    Matrix model_matrix = Matrix::identity();
    Matrix view_matrix = camera.lookat(eye_position, center, up);
    Matrix proj_matrix = camera.proj(eye_position, center);
    Matrix viewport_matrix = camera.viewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4, depth);

    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    SimpleShader head_shader;
    head_shader.setModel(model);
    head_shader.setModelMatrix(model_matrix);
    head_shader.setViewMatrix(view_matrix);
    head_shader.setProjMatrix(proj_matrix);
    head_shader.setViewportMatrix(viewport_matrix);
    head_shader.setLightDir(light_dir);

    for (int i = 0; i < model->nfaces(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = head_shader.vertex(i, j);
        }
        triangle(screen_coords, head_shader, image, zbuffer);
    }

    CubeShader cube_shader;
    cube_shader.setModelMatrix(Matrix::identity());
    cube_shader.setViewMatrix(view_matrix);
    cube_shader.setProjMatrix(proj_matrix);
    cube_shader.setViewportMatrix(viewport_matrix);
    cube_shader.setLightDir(light_dir);
    cube_shader.setColor(TGAColor(200, 230, 255, 100));
    cube_shader.setTransparency(0.5f);
    cube_shader.setCubeGeometry(create_cube_vertices(2.0f), create_cube_faces());

    for (int i = 0; i < cube_shader.getFaceCount(); i++) {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++) {
            screen_coords[j] = cube_shader.vertex(i, j);
        }
        draw_triangle_with_transparency(screen_coords, cube_shader, image, zbuffer);
    }

    image.flip_vertically(); 
    zbuffer.flip_vertically();
    image.write_tga_file("output.tga");
    zbuffer.write_tga_file("zbuffer.tga");

    delete model;
    return 0;
}