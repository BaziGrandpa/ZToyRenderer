#include "head/tgaimage.h"
#include "head/model.h"
#include "head/geometry.h"
#include "head/rasterization.h"
#include <iostream>
#include <cmath>
#include <algorithm>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(51, 204, 51, 255);
const TGAColor pink = TGAColor(255, 153, 255, 255);
const TGAColor orange = TGAColor(255, 153, 102, 255);
const TGAColor blue = TGAColor(0, 102, 255, 255);

//定义光源方向
Vec3f lightDir = Vec3f(0, 0, -1);
Vec3f eye(10, 0, 10);
Vec3f center(0, 0, 0);

Model *model = NULL;
TGAImage *diffuse = NULL;
const int width = 800;
const int height = 800;
const int depth = 255; //生成shadow map的时候刚好作为灰度

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
        Vec3f z = (eye - center).normalize();
        Vec3f x = cross(up, z).normalize(); //这里up和z不必正交，也能算出最终正交的三个基
        Vec3f y = cross(z, x).normalize();
        Matrix Minv = Matrix::identity();
        Matrix Tr = Matrix::identity();
        for (int i = 0; i < 3; i++)
        {
                Minv[0][i] = x[i];
                Minv[1][i] = y[i];
                Minv[2][i] = z[i];
                Tr[i][3] = -eye[i];
        }
        return Minv * Tr;
}

Matrix ViewPort()
{
        Matrix viewport = Matrix::identity();
        viewport[0][0] = width / 2;
        viewport[0][3] = width / 2;
        viewport[1][1] = height / 2;
        viewport[1][3] = height / 2;
        // viewport[2][2] = depth / 2;
        // viewport[2][3] = depth / 2;
        viewport[2][2] = 1;
        viewport[2][3] = 0;
        return viewport;
}
//初始化所有变换矩阵
// 1.Model变换若模型自身要缩放旋转，就先把模型中心点移到世界中心原点，做完变换再移回去，只是数值运算上有这个步骤，并不代表会这样渲染
// 2.View变换，把所有模型变换到摄像机空间，这里是以摄像机中心为原点，一个lookat，up，还是另一个叉乘方向
//      但这里采取的是另一种做法，因为只渲染一个模型，所以可以定义一个中心点，
// 3.Project变换
//      投影到近平面，把frustum变成cube，需要借助远近平面进行矩阵计算
//      也可以简单的全部投影到远平面，这里实现的投影，没有fov的概念，所以就算拉远相机，也就是改变一下透视比例，并不会有明显的远离的感觉
// 4.ViewPort变换，最中从[-1,1]*3变换到屏幕空间
//      宽高乘上(分辨率/2)，再挪到屏幕中心就可以
Matrix InitMatrix()
{
        Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
        Matrix Projection = Matrix::identity(); //是简单的远平面投影，基于相似，不是完整版的project
        Projection[3][2] = -1.f / (eye - center).norm();
        Matrix vp = ViewPort();

        std::cerr << ModelView << std::endl;
        std::cerr << Projection << std::endl;

        Matrix z = (vp * Projection * ModelView);
        std::cerr << z << std::endl;
        return z;
}

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

Vec3f world2screen(Vec3f v)
{
        // return Vec3f(int((v.x + 1.) * (width - 200) / 2. + 100.), int((v.y + 1.) * (height - 200) / 2. + 100.), v.z);
        return Vec3f(int((v.x * (width) / 2 + width / 2)), int((v.y * (height) / 2 + height / 2)), v.z);
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
        //获取mvp变换
        Matrix mvp = InitMatrix();

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
                        Vec3f temp = m2v(mvp * v2m(v));
                        pts[i] = Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题

                        int uvId = i * 3 + 1;
                        Vec3f uv = model->uv(face[uvId]);
                        uvs[i] = uv;

                        int normalId = i * 3 + 2;
                        Vec3f normal = model->normal(face[normalId]);
                        normals[i] = normal;
                }

                RasterizedTiangle4(pts, normals, uvs, zbuffer, image, diffuse, true);
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
        return 0;
}
