using namespace std;
#include "../head/tgaimage.h"
#include "../head/model.h"
#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/shader.h"
#include <cmath>
#include <algorithm>

//�����������ع���shader
//----------------------------------------------------------------------------
//һ������������������Σ���������ֱ��ȡ��i���������͵ĵڼ�������
Vec3f GouraudShader::vertex(Model *model, int faceId, int i)
{
    //�õ�����һ����Ķ�������Id��uvId��normalId
    std::vector<int> face = model->face(faceId);

    //����������ȡuv
    int uvId = i * 3 + 1;
    Vec3f uv = model->uv(face[uvId]);
    uvs[i] = uv;

    //�����編��
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    normals[i] = normal;

    //���������꣬Ȼ����mvp�任
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();
    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������
}

//������һ������������У�����Ϊ��ƬԪ����ɫ
//����ֵΪ�Ƿ���Ⱦ��ƬԪ
bool GouraudShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{

    Vec3f bc_normal = Vec3f(0, 0, 0);
    Vec3f bc_uv = Vec3f(0, 0, 0);
    //����Ļ�ռ�����Ķ���������z��ֵ
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
        bc_uv = bc_uv + uvs[i] * bc_screen[i];
    }
    //��������001��⣬�ղ����ľͺ�ɫ
    float intensity = std::max(bc_normal.normalize() * lightDir.normalize(), 0.f);
    TGAImage *diffuse = GetDiffuse();
    int dWidth = diffuse->get_width();
    int dHeight = diffuse->get_height();
    TGAColor color = needDiffuse ? diffuse->get(bc_uv.x * dWidth, (1 - bc_uv.y) * dHeight) : white;
    oColor.r = intensity * color.r;
    oColor.g = intensity * color.g;
    oColor.b = intensity * color.b;
    oColor.a = 255;
    return false;
}
//----------------------------------------------------------------------------


//С��ͨshader
//----------------------------------------------------------------------------
Vec3f ToonShader::vertex(Model *model, int faceId, int i)
{
    //�õ�����һ����Ķ�������Id��uvId��normalId
    std::vector<int> face = model->face(faceId);

    //�����編��
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    normals[i] = normal;

    //���������꣬Ȼ����mvp�任
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();
    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������
}

bool ToonShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    Vec3f bc_normal = Vec3f(0, 0, 0);
    //����Ļ�ռ�����Ķ���������z��ֵ
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
    }
    //�⿴������ȫ��
    float intensity = std::max(bc_normal.normalize() * lightDir.normalize(), 0.f);

    if (intensity >= 0.95)
        intensity = 1;
    else if (intensity >= 0.8)
        intensity = 0.8;
    else if (intensity >= 0.6)
        intensity = 0.6;
    else if (intensity >= 0.4)
        intensity = 0.4;
    else if (intensity >= 0.2)
        intensity = 0.2;
    oColor = TGAColor(orange.r * intensity, orange.g * intensity, orange.b * intensity, 255);
    return false;
}
//----------------------------------------------------------------------------
