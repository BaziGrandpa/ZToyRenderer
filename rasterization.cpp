#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "rasterization.h"

//ֱ�ߵĹ�դ��
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    //���ȱ���x1>x2
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    bool isSteep = false;
    // y���������y�����
    if (y1 - y0 > x1 - x0)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
        isSteep = true;
    }
    //�������ʽ���ĸ��᳤�Ͷ��ĸ������
    float k = (float)(y1 - y0) / (x1 - x0);
    float b = y1 - k * x1;
    int xLength = x1 - x0;
    // y = k*x +b
    for (int i = 0; i <= xLength; i++)
    {
        /* code */
        int x = (x0 + i);
        int y = k * x + b;
        //ֻ��set int ���Բ�������1�Ѿ��㹻��
        if (isSteep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}

// todo �����c++���ú�ָ��Ĵ���
//ֻ�ǽ����˼򵥵Ķ�λλ�ƺ����ű任�����Ƽ򵥵�����
void RenderWireframe(Model *model, TGAImage &image)
{
    //����������
    for (int i = 0; i < model->nfaces(); i++)
    {
        //��ǰ��
        std::vector<int> face = model->face(i);
        //ȡ��������
        for (int j = 0; j < 7; j += 3)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            //�򵥵ı任����Ϊobj�е����궼�ǹ�һ����
            //�ȴ�[-1,1]3 �任��[0,2]3�ռ�
            // int x0 = (v0.x + 1.) * width / 2;
            // int y0 = (v0.y + 1.) * height / 2;
            // int x1 = (v1.x + 1.) * width / 2;
            // int y1 = (v1.y + 1.) * height / 2;
            // line(x0, y0, x1, y1, image, white);
        }
    }
}

//���Ƽ򵥵�������
void RenderSimpleFragment(Model *model)
{
    for (int i = 0; i < model->nfaces(); i++)
    {
        //��ǰ��
        std::vector<int> face = model->face(i);
        //ȡ����������
        Vec3f v0 = model->vert(face[0]);
        Vec3f v1 = model->vert(face[3]);
        Vec3f v2 = model->vert(face[6]);
        //���򣬰���x������
    }
}

//���������������С
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

//��Ե��������ͣ�����Ļ����ϵ�¹�դ��
// 1��������Χ��
// 2���Զ��㰴��x�����Եڶ�������Ϊ�ֽ�
//      ���ڶ������ڵ�������֮��
//              �ڶ�����࣬<1&&>3
//              �ڶ����Ҳ࣬<2&&<3
//      ���ڶ������ڵ�������֮��
//              ���෴�ж�
void RasterizedTiangle1(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color)
{
    //��Χ��
    int minY = FindMin(v0.y, v1.y, v2.y);
    int maxY = FindMax(v0.y, v1.y, v2.y);

    //����
    if (v0.x > v1.x)
        std::swap(v0, v1);
    if (v1.x > v2.x)
        std::swap(v1, v2);
    if (v0.x > v1.x)
        std::swap(v0, v1);

    //б�ʼ���
    float slope01 = (float)(v0.y - v1.y) / (v0.x - v1.x);
    float slope02 = (float)(v0.y - v2.y) / (v0.x - v2.x);
    if (slope01 == slope02)
        return;
    float slope12 = (float)(v1.y - v2.y) / (v1.x - v2.x);
    //�ؾ����
    float b01 = v0.y - slope01 * v0.x;
    float b02 = v0.y - slope02 * v0.x;
    float b12 = v1.y - slope12 * v1.x;
    //�������¹�դ��
    for (int y = maxY; y >= minY; y--)
    {
        //��դ�����������
        for (int x = v0.x; x <= v1.x; x++)
        {
            //����
            float y01 = slope01 * x + b01;
            float y02 = slope02 * x + b02;

            bool inTriangle = slope02 > slope01 ? (y <= y02 && y >= y01) : (y <= y01 && y >= y02);
            if (inTriangle)
                image.set(x, y, color);
        }
        //��դ���ұ�
        for (int x = v1.x; x <= v2.x; x++)
        {
            //����
            float y12 = slope12 * x + b12;
            float y02 = slope02 * x + b02;

            bool inTriangle = slope02 > slope12 ? (y <= y12 && y >= y02) : (y <= y02 && y >= y12);
            if (inTriangle)
                image.set(x, y, color);
        }
    }
}

//ʹ��������˼·ȥ��դ��������Ҫ��Χ���ˣ�ֱ���������м����
void RasterizedTiangle2(Vec2i v0, Vec2i v1, Vec2i v2, TGAImage &image, TGAColor color)
{
    //����
    if (v0.x > v1.x)
        std::swap(v0, v1);
    if (v1.x > v2.x)
        std::swap(v1, v2);
    if (v0.x > v1.x)
        std::swap(v0, v1);
    int totalWidth = v2.x - v0.x;
    int lSegmentWidth = v1.x - v0.x + 1; //��̱߳���
    int rSegmentWidth = v2.x - v1.x + 1; //�Ҷ̱߳���
    if (totalWidth <= 1)
        return;

    //���ߣ��������һ�
    for (int x = v0.x; x <= v1.x; x++)
    {
        //����������
        float alpha = (float)(x - v0.x) / lSegmentWidth;
        float beta = (float)(x - v0.x) / totalWidth;
        //��������ʽ�ݽ�
        Vec2i segmentP = v0 + (v1 - v0) * alpha;
        Vec2i totalP = v0 + (v2 - v0) * beta;
        int start = v1.y > v2.y ? totalP.y : segmentP.y;
        int end = v1.y > v2.y ? segmentP.y : totalP.y;
        for (int y = start; y <= end; y++)
        {
            //��ɫ
            image.set(x, y, color);
        }
    }

    //�Ұ�ߣ���������
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
            //��ɫ
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

//��׼��Ч�ʵ����������ð�Χ��+������������жϣ����ص��Ƿ����������ڲ�
//��1-u-v��u��v���������߶�����0ʱ��������P���������ڲ�
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

//������zbuffer
//���ڹ�դ��ʱ��zֵ��ȷ��������z���Ǹ�����Ļ�ռ�������Σ�ʹ�����������z���������������ֵ�ģ�������������Ϳ������ܶ��ֵ�ˣ�
void RasterizedTiangle4(Vec3f *pts, Vec3f *normals, float *zbuffer, TGAImage &image, TGAColor color)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    int width = image.get_width();
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
            P.z = 0;
            Vec3f bc_normal = Vec3f(0, 0, 0);
            //����Ļ�ռ�����Ķ���������z��ֵ
            for (int i = 0; i < 3; i++)
            {
                P.z += pts[i].z * bc_screen[i];
                bc_normal = bc_normal + normals[i] * bc_screen[i];
            }
            //����zbuffer
            if (P.z > zbuffer[(int)(P.y * width + P.x)])
            {
                zbuffer[(int)(P.y * width + P.x)] = P.z;
                //�������
                float intensity = bc_normal.normalize() * Vec3f(0, 0, -1);
                image.set(P.x, P.y, TGAColor(intensity * color.r, intensity * color.g, intensity * color.b, 255));
            }
        }
    }
}

//��������
void RasterizedTiangle5(Vec3f *pts, float *zbuffer, TGAImage &image, TGAImage &diffuse, TGAColor color)
{
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    int width = image.get_width();
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
            P.z = 0;
            //����Ļ�ռ�����Ķ���������z��ֵ
            for (int i = 0; i < 3; i++)
                P.z += pts[i].z * bc_screen[i];
            //����zbuffer
            if (P.z > zbuffer[(int)(P.y * width + P.x)])
            {
                zbuffer[(int)(P.y * width + P.x)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}