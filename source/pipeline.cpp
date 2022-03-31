#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/rasterization.h"
#include "../head/model.h"
#include <algorithm>
#include <cmath>
#include <iostream>

const int depth = 255; //生成shadow map的时候刚好作为灰度
float *zbuffer;        //管线中的zbuffer
int widthp = 0;
int heightp = 0;

Model *model = NULL;
TGAImage *diffuse = NULL;
Matrix mvp;

//加载模型
void LoadData(const char *objName, const char *diffuseName)
{

    TGAImage a(100, 100, TGAImage::RGB);
    //读模型
    model = new Model(objName);

    //读贴图
    diffuse = new TGAImage();
    diffuse->read_tga_file(diffuseName);
}

//初始化管线
// 1.初始化输出分辨率
// 2.zbuffer
void InitPipeline(int w, int h)
{
    widthp = w;
    heightp = h;
    zbuffer = new float[widthp * heightp];
    for (int i = widthp * heightp; i--; zbuffer[i] = -std::numeric_limits<float>::max())
        ;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize(); //这里up和z不必正交，也能算出最终正交的三个基，要的是一个平面即可
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

Matrix ViewPort(int width, int height)
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
void InitMatrix(Vec3f eye, Vec3f center)
{
    Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
    Matrix Projection = Matrix::identity(); //是简单的远平面投影，基于相似，不是完整版的project
    Projection[3][2] = -1.f / (eye - center).norm();
    Matrix vp = ViewPort(widthp, heightp);

    std::cerr << ModelView << std::endl;
    std::cerr << Projection << std::endl;

    Matrix z = (vp * Projection * ModelView);
    std::cerr << z << std::endl;
    mvp = z;
}

Matrix GetMVP()
{
    return mvp;
}

//矩阵转向量，最终放到geometry里
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

//获取漫反射贴图，用于fragment中采样
TGAImage *GetDiffuse()
{
    return diffuse;
}

//完成准备工作开始渲染
void Render(Shader *shader)
{
    if (model == NULL)
    {
        std::cout << "load model failed.";
    }
    TGAImage output(widthp, heightp, TGAImage::RGB);
    //一个循环光栅化一个三角面
    for (int faceId = 0; faceId < model->nfaces(); faceId++)
    {
        //一个面的三个顶点经过顶点着色器处理后的坐标，一般来说是屏幕坐标
        Vec3f screenCor[3];
        for (int i = 0; i < 3; i++)
        {
            screenCor[i] = shader->vertex(model, faceId, i);
        }
        RasterizeWithShader(shader, screenCor, zbuffer, output);
    }
    delete model;
    output.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    output.write_tga_file("output.tga");
}