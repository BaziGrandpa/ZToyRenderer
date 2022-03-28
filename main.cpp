#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "rasterization.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(51, 204, 51, 255);
const TGAColor pink = TGAColor(255, 153, 255, 255);
const TGAColor orange = TGAColor(255, 153, 102, 255);
const TGAColor blue = TGAColor(0, 102, 255, 255);

//定义光源方向
Vec3f lightDir = Vec3f(0, 0, -1);
Vec3f camera(0, 0, 5);

Model *model = NULL;
TGAImage *diffuse = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

//矩阵转向量
Vec3f m2v(Matrix m)
{
        return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v)
{
        Matrix m = Matrix();
        m[0][0] = v.x;
        m[1][0] = v.y;
        m[2][0] = v.z;
        m[3][0] = 1.f;
        return m;
}

Matrix viewport(int x, int y, int w, int h)
{
        Matrix m = Matrix::identity();
        m[0][3] = x + w / 2.f;
        m[1][3] = y + h / 2.f;
        m[2][3] = depth / 2.f;

        m[0][0] = w / 2.f;
        m[1][1] = h / 2.f;
        m[2][2] = depth / 2.f;
        return m;
}

Vec3f world2screen(Vec3f v)
{
        return Vec3f(int((v.x + 1.) * (width - 200) / 2. + 100.), int((v.y + 1.) * (height - 200) / 2. + 100.), v.z);
}
int main(int argc, char **argv)
{
        //读模型
        model = new Model("obj/african_head.obj");

        //读贴图
        diffuse = new TGAImage();
        diffuse->read_tga_file("texture/african_head_diffuse.tga");

        float *zbuffer = new float[width * height];
        for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max())
                ;

        TGAImage image(width, height, TGAImage::RGB);

        //视口变换相关
        Matrix Projection = Matrix::identity();
        // Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        Projection[3][2] = -1.f / camera.z;

        for (int i = 0; i < model->nfaces(); i++)
        {
                std::vector<int> face = model->face(i);
                Vec3f pts[3];
                Vec3f worldPos[3]; // obj原始世界坐标
                Vec3f normals[3];
                Vec3f uvs[3];
                for (int i = 0; i < 3; i++) //一个面的三个顶点
                {
                        int vertexId = i * 3;
                        Vec3f v = model->vert(face[vertexId]);
                        // pts[i] = world2screen(v);
                        Matrix temp0 = v2m(v);
                        Matrix temp = Projection * temp0;
                        pts[i] = world2screen(m2v(temp));
                        // worldPos[i] = Vec3f(v.x, v.y, v.z);

                        int uvId = i * 3 + 1;
                        Vec3f uv = model->uv(face[uvId]);
                        uvs[i] = uv;

                        int normalId = i * 3 + 2;
                        Vec3f normal = model->normal(face[normalId]);
                        normals[i] = normal;
                }
                //法线
                // Vec3f normal = cross(worldPos[2] - worldPos[0], worldPos[1] - worldPos[0]);
                // normal.normalize();
                // float intensity = normal * lightDir;
                // if (intensity > 0)
                //         RasterizedTiangle4(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
                RasterizedTiangle4(pts, normals, uvs, zbuffer, image, diffuse);
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
        return 0;
}
