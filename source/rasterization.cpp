#include "../head/tgaimage.h"
#include "../head/model.h"
#include "../head/geometry.h"
#include "../head/rasterization.h"
#include "../head/pipeline.h"
#include <iostream>
#include <cmath>
#include <algorithm>

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
        for (int j = 0; j < 7; j += 3)
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
        Vec3f v1 = model->vert(face[3]);
        Vec3f v2 = model->vert(face[6]);
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

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
    Vec3f s[2];
    for (int i = 2; i--;)
    {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

//标准高效率的做法，利用包围盒+重心坐标进行判断，像素点是否在三角型内部
//（1-u-v，u，v）仅当三者都大于0时，给定点P在三角型内部
void RasterizedTiangle3(Vec3f *pts, TGAImage &image, TGAColor color)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            image.set(P.x, P.y, color);
        }
    }
}

//增加了zbuffer
//关于光栅化时，z值的确定，这里z，是根据屏幕空间的三角形，使用世界坐标的z，进行重心坐标插值的，有了重心坐标就可以做很多插值了！
//传递进来的pts，是屏幕空间的坐标了（除了z），
void RasterizedTiangle4(Vec3f *pts, Vec3f *normals, Vec3f *uvs, float *zbuffer, TGAImage &image, TGAImage &diffuse, Vec3f &lightDir, bool needDiffuse)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    int width = image.get_width();
    int height = image.get_height();

    int dWidth = diffuse.get_width();
    int dHeight = diffuse.get_height();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    //在包围盒里做光栅化
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;
            P.z = 0;
            Vec3f bc_normal = Vec3f(0, 0, 0);
            Vec3f bc_uv = Vec3f(0, 0, 0);
            //用屏幕空间的重心对世界坐标z插值
            for (int i = 0; i < 3; i++)
            {
                P.z += pts[i].z * bc_screen[i];
                bc_normal = bc_normal + normals[i] * bc_screen[i];
                bc_uv = bc_uv + uvs[i] * bc_screen[i];
            }
            //假设正面001打光，照不到的就黑色
            float intensity = std::max(bc_normal.normalize() * lightDir.normalize(), 0.f);
            // float intensity = cross(pts[1] - pts[0], pts[2] - pts[0]).normalize() * Vec3f(0, 0, 1);
            //更新zbuffer,无光的直接丢弃，这里的zbuffer也就默认了，z值越大的越先渲染，也就无形中将摄像机摆在了z轴正向
            if (P.z > zbuffer[(int)(P.y * width + P.x)])
            {
                zbuffer[(int)(P.y * width + P.x)] = P.z;
                //用uv采样贴图
                TGAColor color = needDiffuse ? diffuse.get(bc_uv.x * dWidth, (1 - bc_uv.y) * dHeight) : TGAColor(255, 255, 255, 255);
                image.set(P.x, P.y, TGAColor(intensity * color.r, intensity * color.g, intensity * color.b, 255));
            }
        }
    }
}
void RasterizeWithShader(Shader *Shader, Vec3f *screenCord, float *zbuffer, TGAImage &image, bool ssaa)
{
    //构建包围盒
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    int width = image.get_width();
    int height = image.get_height();
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], screenCord[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], screenCord[i][j]));
        }
    }
    //当前正在光栅化的像素
    Vec3f P;

    //在包围盒里做光栅化
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            Vec3f bc_screen = barycentric(screenCord[0], screenCord[1], screenCord[2], P);
            if (ssaa)
            {
                //因为一个像素又分为四个，所以不好判断是否在三角型内了
                Vec3f ms_bc_screen;
                float r = 0;
                float g = 0;
                float b = 0;
                //记录四个采样点，有没有一个是在三角形内的，有一个在就为true
                bool inside = false;
                //一个像素点周围采样四次
                for (int i = 1; i <= 4; i++)
                {
                    float xStep = (((i % 2) * 2) - 1) * -0.5; // i==1,3时 xStep==-0.5 i==2,4时xStep==0.5
                    float yStep = (((i % 2) * 2) - 1) * 0.5;  // i==1,3时 yStep==0.5 i==2,4时xStep==-0.5
                    //构造超采样点
                    Vec3f ms_P(P.x + xStep, P.y + yStep, 0);
                    ms_bc_screen = barycentric(screenCord[0], screenCord[1], screenCord[2], ms_P);
                    bool curInside = ms_bc_screen.x >= 0 && ms_bc_screen.y >= 0 && ms_bc_screen.z >= 0; //超采样点是否在三角型内
                    TGAColor ms_color(255, 255, 255, 255);                                              //边缘和白色混合
                    if (curInside)
                    {
                        Shader->fragment(ms_bc_screen, ms_color);
                        inside = true;
                    }
                    r += ms_color.r;
                    g += ms_color.g;
                    b += ms_color.b;
                    //不在三角型内，就算是+ms_color了
                }

                //这点是需要着色的，因为四个采样点有一个是在三角形内
                if (inside)
                {
                    //到这里再深度测试，注意这里使用中心点的
                    float z = screenCord[0][2] * bc_screen.x + screenCord[1][2] * bc_screen.y + screenCord[2][2] * bc_screen.z;
                    if (z > zbuffer[(int)(P.y * width + P.x)])
                    {
                        zbuffer[(int)(P.y * width + P.x)] = z;
                        unsigned char cr = r / 4;
                        unsigned char cg = g / 4;
                        unsigned char cb = b / 4;
                        TGAColor ssaaColor(cr, cg, cb, 255);
                        image.set(P.x, P.y, ssaaColor);
                    }
                }
            }
            else
            {
                //求出重心坐标

                if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                    continue;
                //直接让框架计算z值
                float z = screenCord[0][2] * bc_screen.x + screenCord[1][2] * bc_screen.y + screenCord[2][2] * bc_screen.z;
                //更新zbuffer,无光的直接丢弃，这里的zbuffer也就默认了，z值越大的越先渲染，也就无形中将摄像机摆在了z轴正向
                //因为zbuffer是在相机空间更新的，所以如果把相机摆在-z，那么下面的判断应该改成小于号
                if (z > zbuffer[(int)(P.y * width + P.x)])
                {
                    zbuffer[(int)(P.y * width + P.x)] = z;

                    TGAColor fragColor;
                    //如果片元着色器决定丢弃，最终不会着色
                    if (!Shader->fragment(bc_screen, fragColor))
                    {
                        image.set(P.x, P.y, fragColor);
                    }
                }
            }
        }
    }
}
