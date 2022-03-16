#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include"rasterization.h"

//直线的光栅化
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
                        // int x0 = (v0.x + 1.) * width / 2;
                        // int y0 = (v0.y + 1.) * height / 2;
                        // int x1 = (v1.x + 1.) * width / 2;
                        // int y1 = (v1.y + 1.) * height / 2;
                        // line(x0, y0, x1, y1, image, white);
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

//使用向量的思路去光栅化，不需要包围盒了，直接在两边中间填充
void RasterizedTiangle2(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color)
{
        //排序
        if (v0.x > v1.x)
                std::swap(v0, v1);
        if (v1.x > v2.x)
                std::swap(v1, v2);
        if (v0.x > v1.x)
                std::swap(v0, v1);
        int totalWidth = v2.x - v0.x;
        int lSegmentWidth = v1.x - v0.x + 1; //左短边长度
        int rSegmentWidth = v2.x - v1.x + 1; //右短边长度
        if (totalWidth <= 1)
                return;

        //左半边，从左往右画
        for (int x = v0.x; x <= v1.x; x++)
        {
                //计算横向比例
                float alpha = (float)(x - v0.x) / lSegmentWidth;
                float beta = (float)(x - v0.x) / totalWidth;
                //以向量形式递进
                Vec2i segmentP = v0 + (v1 - v0) * alpha;
                Vec2i totalP = v0 + (v2 - v0) * beta;
                int start = v1.y > v2.y ? totalP.y : segmentP.y;
                int end = v1.y > v2.y ? segmentP.y : totalP.y;
                for (int y = start; y <= end; y++)
                {
                        //着色
                        image.set(x, y, color);
                }
        }

        //右半边，从右往左画
        for (int x = v2.x; x >= v1.x; x--)
        {
                float alpha = (float)(v2.x - x) / rSegmentWidth;
                float beta = (float)(v2.x - x) / totalWidth;
                Vec2i segmentP = v2 + (v1 - v2) * alpha;
                Vec2i totalP = v2 + (v0 - v2) * beta;
                int start = v1.y > v2.y ? totalP.y : segmentP.y;
                int end = v1.y > v2.y ? segmentP.y : totalP.y;
                for (int y = start; y <= end; y++)
                {
                        //着色
                        image.set(x, y, color);
                }
        }
}

//利用叉乘求重心坐标
Vec3f barycentric(Vec2i *pts, Vec2i P)
{
        Vec3f u = Vec3f(pts[2].raw[0] - pts[0].raw[0], pts[1].raw[0] - pts[0].raw[0], pts[0].raw[0] - P.raw[0]) ^ Vec3f(pts[2].raw[1] - pts[0].raw[1], pts[1].raw[1] - pts[0].raw[1], pts[0].raw[1] - P.raw[1]);
        /* `pts` and `P` has integer value as coordinates
           so `abs(u[2])` < 1 means `u[2]` is 0, that means
           triangle is degenerate, in this case return something with negative coordinates */
        if (std::abs(u.z) < 1)
                return Vec3f(-1, 1, 1);
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

//标准高效率的做法，利用包围盒+重心坐标进行判断，像素点是否在三角型内部
//（1-u-v，u，v）仅当三者都大于0时，给定点P在三角型内部
void RasterizedTiangle3(Vec2i *pts, TGAImage &image, TGAColor color)
{
        Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
        Vec2i bboxmax(0, 0);
        Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
        for (int i = 0; i < 3; i++)
        {
                bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
                bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

                bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
                bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
        }
        Vec2i P;
        for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
        {
                for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
                {
                        Vec3f bc_screen = barycentric(pts, P);
                        if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                                continue;
                        image.set(P.x, P.y, color);
                }
        }
}
