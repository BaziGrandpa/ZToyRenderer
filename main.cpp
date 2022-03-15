#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(51, 204, 51, 255);
const TGAColor pink = TGAColor(255, 153, 255, 255);
const TGAColor orange = TGAColor(255, 153, 102, 255);
const TGAColor blue = TGAColor(0, 102, 255, 255);

Model *model = NULL;
const int width = 200;
const int height = 200;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
        //首先保障x1>x2
        if (x1 < x0)
        {
                std::swap(x0, x1);
                std::swap(y0, y1);
        }

        bool isSteep = false;
        // y轴更长，对y轴采样
        if (y1 - y0 > x1 - x0)
        {
                std::swap(x0, y0);
                std::swap(x1, y1);
                isSteep = true;
        }
        //建立表达式，哪个轴长就对哪个轴采样
        float k = (float)(y1 - y0) / (x1 - x0);
        float b = y1 - k * x1;
        int xLength = x1 - x0;
        // y = k*x +b
        for (int i = 0; i <= xLength; i++)
        {
                /* code */
                int x = (x0 + i);
                int y = k * x + b;
                //只能set int 所以采样精度1已经足够了
                if (isSteep)
                        image.set(y, x, color);
                else
                        image.set(x, y, color);
        }
}

// todo 搞清楚c++引用和指针的传递
//只是进行了简单的二位位移和缩放变换，绘制简单的网格
void RenderWireframe(Model *model, TGAImage &image)
{
        //根据网格画线
        for (int i = 0; i < model->nfaces(); i++)
        {
                //当前面
                std::vector<int> face = model->face(i);
                //取三个顶点
                for (int j = 0; j < 3; j++)
                {
                        Vec3f v0 = model->vert(face[j]);
                        Vec3f v1 = model->vert(face[(j + 1) % 3]);
                        //简单的变换，因为obj中的坐标都是归一化的
                        //先从[-1,1]3 变换到[0,2]3空间
                        int x0 = (v0.x + 1.) * width / 2;
                        int y0 = (v0.y + 1.) * height / 2;
                        int x1 = (v1.x + 1.) * width / 2;
                        int y1 = (v1.y + 1.) * height / 2;
                        line(x0, y0, x1, y1, image, white);
                }
        }
}

//绘制简单的三角面
void RenderSimpleFragment(Model *model)
{
        for (int i = 0; i < model->nfaces(); i++)
        {
                //当前面
                std::vector<int> face = model->face(i);
                //取出三个顶点
                Vec3f v0 = model->vert(face[0]);
                Vec3f v1 = model->vert(face[1]);
                Vec3f v2 = model->vert(face[2]);
                //排序，按照x轴坐标
        }
}

//三个数字找最大最小
int FindMax(int i1, int i2, int i3)
{
        int min = i1 > i2 ? i1 : i2;
        return min > i3 ? min : i3;
}
int FindMin(int i1, int i2, int i3)
{
        int min = i1 < i2 ? i1 : i2;
        return min < i3 ? min : i3;
}

//针对单个三角型，在屏幕坐标系下光栅化
// 1，建立包围盒
// 2，对顶点按照x排序，以第二个顶点为分界
//      若第二顶点在第三顶点之上
//              在顶点左侧，<1&&>3
//              在顶点右侧，<2&&<3
//      若第二顶点在第三顶点之下
//              则相反判断
void RasterizedTiangle1(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color)
{
        //包围盒
        int minY = FindMin(v0.y, v1.y, v2.y);
        int maxY = FindMax(v0.y, v1.y, v2.y);

        //排序
        if (v0.x > v1.x)
                std::swap(v0, v1);
        if (v1.x > v2.x)
                std::swap(v1, v2);
        if (v0.x > v1.x)
                std::swap(v0, v1);

        //斜率计算
        float slope01 = (float)(v0.y - v1.y) / (v0.x - v1.x);
        float slope02 = (float)(v0.y - v2.y) / (v0.x - v2.x);
        if (slope01 == slope02)
                return;
        float slope12 = (float)(v1.y - v2.y) / (v1.x - v2.x);
        //截距计算
        float b01 = v0.y - slope01 * v0.x;
        float b02 = v0.y - slope02 * v0.x;
        float b12 = v1.y - slope12 * v1.x;
        //从上往下光栅化
        for (int y = maxY; y >= minY; y--)
        {
                //光栅化三角型左边
                for (int x = v0.x; x <= v1.x; x++)
                {
                        //采样
                        float y01 = slope01 * x + b01;
                        float y02 = slope02 * x + b02;

                        bool inTriangle = slope02 > slope01 ? (y <= y02 && y >= y01) : (y <= y01 && y >= y02);
                        if (inTriangle)
                                image.set(x, y, color);
                }
                //光栅化右边
                for (int x = v1.x; x <= v2.x; x++)
                {
                        //采样
                        float y12 = slope12 * x + b12;
                        float y02 = slope02 * x + b02;

                        bool inTriangle = slope02 > slope12 ? (y <= y12 && y >= y02) : (y <= y02 && y >= y12);
                        if (inTriangle)
                                image.set(x, y, color);
                }
        }
}

int main(int argc, char **argv)
{
        //读模型
        // model = new Model("obj/african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);

        Vec2i t0[3] = {Vec2i(20, 20), Vec2i(100, 100), Vec2i(20, 180)};
        Vec2i t1[3] = {Vec2i(20, 180), Vec2i(100, 100), Vec2i(180, 180)};
        Vec2i t2[3] = {Vec2i(180, 180), Vec2i(100, 100), Vec2i(180, 20)};

        RasterizedTiangle1(t0[0], t0[1], t0[2], image, blue);
        RasterizedTiangle1(t1[0], t1[1], t1[2], image, red);
        RasterizedTiangle1(t2[0], t2[1], t2[2], image, green);
        // RenderWireframe(model, image);
        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
        return 0;
}
