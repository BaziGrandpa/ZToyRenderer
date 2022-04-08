#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/rasterization.h"
#include "../head/model.h"
#include <algorithm>
#include <cmath>
#include <iostream>

const int depth = 255; //����shadow map��ʱ��պ���Ϊ�Ҷ�
float *zbuffer;        //�����е�zbuffer
int widthp = 0;
int heightp = 0;

Model *model = NULL;
TGAImage *diffuse = NULL;
Matrix mvp;
Matrix shadowMVP; // shadowmap�ռ�任

//����ģ��
void LoadData(const char *objName, const char *diffuseName)
{

    TGAImage a(100, 100, TGAImage::RGB);
    //��ģ��
    model = new Model(objName);

    //����ͼ
    diffuse = new TGAImage();
    if (!diffuse->read_tga_file(diffuseName))
        diffuse = nullptr;
}

//��ʼ������
// 1.��ʼ������ֱ���
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
    Vec3f x = cross(up, z).normalize(); //����up��z����������Ҳ�����������������������Ҫ����һ��ƽ�漴��
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
    //���ת�����ԭ����center
    return Minv * Tr;
}

Matrix ViewPort(int width, int height)
{
    Matrix viewport = Matrix::identity();
    viewport[0][0] = width / 2;
    viewport[0][3] = width / 2;
    viewport[1][1] = height / 2;
    viewport[1][3] = height / 2;
    viewport[2][2] = 1;
    viewport[2][3] = 2; //��zֵ��-1��1 �任��0 1
    viewport[2][2] = 1;
    viewport[2][3] = 0;
    return viewport;
}
//��ʼ�����б任����
// 1.Model�任��ģ������Ҫ������ת�����Ȱ�ģ�����ĵ��Ƶ���������ԭ�㣬����任���ƻ�ȥ��ֻ����ֵ��������������裬���������������Ⱦ
// 2.View�任��������ģ�ͱ任��������ռ䣬�����������Ϊԭ�㣬һ��lookat��up��������һ����˷���
//      �������ȡ������һ����������Ϊֻ��Ⱦһ��ģ�ͣ����Կ��Զ���һ�����ĵ㣬
// 3.Project�任
//      ͶӰ����ƽ�棬��frustum���cube����Ҫ����Զ��ƽ����о������
//      Ҳ���Լ򵥵�ȫ��ͶӰ��Զƽ�棬����ʵ�ֵ�ͶӰ��û��fov�ĸ�����Ծ�����Զ�����Ҳ���Ǹı�һ��͸�ӱ����������������Ե�Զ��ĸо�
// 4.ViewPort�任�����д�[-1,1]*3�任����Ļ�ռ�
//      ��߳���(�ֱ���/2)����Ų����Ļ���ľͿ���
void InitMatrix(Vec3f eye, Vec3f lightDir, Vec3f center)
{
    Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
    Matrix Projection = Matrix::identity(); //�Ǽ򵥵�Զƽ��ͶӰ���������ƣ������������project
    Projection[3][2] = -1.f / (eye - center).norm();
    Matrix vp = ViewPort(widthp, heightp);

    std::cerr << ModelView << std::endl;
    std::cerr << Projection << std::endl;

    Matrix z = (vp * Projection * ModelView);
    std::cerr << z << std::endl;
    mvp = z;

    // shadowmap����
    Matrix ShadowModelView = lookat(lightDir, center, Vec3f(0, 1, 0));
    shadowMVP = vp * Projection * ShadowModelView;
}

Matrix GetMVP()
{
    return mvp;
}
Matrix GetShadowMVP()
{
    return shadowMVP;
}

//����ת���������շŵ�geometry��
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

//��ȡ��������ͼ������fragment�в���
TGAImage *GetDiffuse()
{
    return diffuse;
}

//����ڸ÷���ƬԪ������Ƕȣ����������ƽ�����arctan�㣬�û����Ʊ�ʾ
float max_elevation_angle(Vec2f p, Vec2f dir)
{
    float maxangle = 0;
    for (float t = 0.; t < 10.; t += 1.)
    {
        Vec2f cur = p + dir * t;
        if (cur.x >= widthp || cur.y >= heightp || cur.x < 0 || cur.y < 0)
            return maxangle;

        float distance = (p - cur).norm();
        if (distance < 1.f)
            continue;
        float elevation = zbuffer[int(cur.x) + int(cur.y) * widthp] - zbuffer[int(p.x) + int(p.y) * widthp];
        maxangle = std::max(maxangle, atanf(elevation / distance));
    }
    return maxangle;
}
float EvaluateSSAOIntensity(int x, int y)
{
    float PI = 3.14;
    float total = 0;
    //��8��������̽
    for (float a = 0; a < PI * 2 - 1e-4; a += PI / 4)
    {
        //��PI/2���Ƕȣ���Ϊ���ýǶ�Խ��ģ�totalԽС�����ϵ��С����ɫ���ڣ����Ƕ�Խ��õ�Խ�ڣ�
        total += PI / 2 - max_elevation_angle(Vec2f(x, y), Vec2f(cos(a), sin(a)));
    }
    total /= (PI / 2) * 8;     //��total��һ������Ϊ���ÿ��maxangle����0����total��ȥ��(PI/2)*8
    total = pow(total, 100.f); //�͸߹⴦��һ�����÷����Ժ����Ŵ�Ч��
    return total;
}

//���׼��������ʼ��Ⱦ
void Render(Shader *shader)
{
    if (model == NULL)
    {
        std::cout << "load model failed.";
    }
    TGAImage output(widthp, heightp, TGAImage::RGB);

    //һ��ѭ����դ��һ��������
    for (int faceId = 0; faceId < model->nfaces(); faceId++)
    {
        //һ������������㾭��������ɫ�����������꣬һ����˵����Ļ����
        Vec3f screenCor[3];
        for (int i = 0; i < 3; i++)
        {
            screenCor[i] = shader->vertex(model, faceId, i);
        }
        //��ʵ���ﻹȱ��һ������Ҫ���任��������ͣ��Ƿ�����Ļ�����ڣ�Ȼ������Ļ�߽����������Ҫ�ü�
        // todo check if triangle in screen

        RasterizeWithShader(shader, screenCor, zbuffer, output, false);
    }
    delete model;
    //��Ļ����
    // ssao
    // output.clear();
    // output = TGAImage(widthp, heightp, TGAImage::RGB);
    // for (int x = 0; x < widthp; x++)
    // {
    //     for (int y = 0; y < heightp; y++)
    //     {
    //         int bufferId = x + y * widthp;
    //         float zValue = zbuffer[bufferId];
    //         float min = -100000;
    //         if (zValue < min) //û��fragment
    //             continue;

    //         float intensity = EvaluateSSAOIntensity(x, y);
    //         output.set(x, y, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
    //     }
    // }

    output.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    output.write_tga_file("output.tga");
}