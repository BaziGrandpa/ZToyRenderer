using namespace std;
#include "../head/tgaimage.h"
#include "../head/model.h"
#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/shader.h"
#include <cmath>
#include <algorithm>

//�ڶ��������ɫ��fragmentֻ�Ǽ򵥵Ĳ�ֵ
//----------------------------------------------------------------------------
Vec3f GouraudShader::vertex(Model *model, int faceId, int i)
{
    //�õ�����һ����Ķ�������Id��uvId��normalId
    std::vector<int> face = model->face(faceId);

    //�����編��
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    varing_intensity[i] = max(.0f, normal.normalize() * lightDir.normalize());

    //���������꣬Ȼ����mvp�任
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();
    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������
}

//����ƬԪ��ɫһ��
bool GouraudShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    float bc_intensity = min(varing_intensity * Vec3f(0.33, 0.33, 0.33), 1.f);
    oColor = TGAColor(white.r * bc_intensity, white.g * bc_intensity, white.b * bc_intensity, 255);
    return false;
}
//----------------------------------------------------------------------------

//��������������ɫshader
//----------------------------------------------------------------------------
//һ������������������Σ���������ֱ��ȡ��i���������͵ĵڼ�������
Vec3f PhongShader::vertex(Model *model, int faceId, int i)
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
bool PhongShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{

    Vec3f bc_normal = Vec3f(0, 0, 0);
    Vec3f bc_uv = Vec3f(0, 0, 0);
    //����Ļ�ռ�����Ķ���������z��ֵ
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
        bc_uv = bc_uv + uvs[i] * bc_screen[i];
    }
    //�ղ����ľͺ�ɫ��
    //�������֮���Կ�����ȷ����Ϊ��ȫ�������������µļ��㣬�൱�ڰ�ģ��һֱ��������ԭ��ȥ������
    //��ͨ�õ�������Ӧ���ǰѵƱ任�����߿ռ䣬��normalmap��������
    float intensity = std::max(bc_normal.normalize() * lightDir.normalize(), 0.f);
    intensity = min(intensity + 0.1f, 1.f);
    TGAImage *diffuse = GetDiffuse();
    if (diffuse == nullptr) //û����������ǿ����false
        needDiffuse = false;

    TGAColor color = needDiffuse ? diffuse->get(bc_uv.x * diffuse->get_width(), (1 - bc_uv.y) * diffuse->get_height()) : white;
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

//�򵥵�֧�ָ߹�shader
//----------------------------------------------------------------------------
BlinnPhongShader::BlinnPhongShader()
{
    //���۲����������ת������Ļ�ռ�
    Matrix mvp = GetMVP();
    screenEye = m2v(mvp * v2m(eye));
    screenLightDir = m2v(mvp * v2m(lightDir));
    InverseMVP = mvp.invert_transpose();
    cout << "eye:" << screenEye << "ld:" << screenLightDir;
}
Vec3f BlinnPhongShader::vertex(Model *model, int faceId, int i)
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

//�Ȳ������߹���ͼ�ˣ�ֱ�ӵ�����ģ�Ͷ��и߹�
bool BlinnPhongShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    Vec3f bc_normal = Vec3f(0, 0, 0);
    Vec3f bc_uv = Vec3f(0, 0, 0);
    //����Ļ�ռ�����Ķ���������z��ֵ
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
        bc_uv = bc_uv + uvs[i] * bc_screen[i];
    }
    //������������ɫ
    TGAImage *diffuse = GetDiffuse();
    if (diffuse == nullptr)
        return true;
    int dWidth = diffuse->get_width();
    int dHeight = diffuse->get_height();
    TGAColor color = diffuse->get(bc_uv.x * dWidth, (1 - bc_uv.y) * dHeight);

    //������ռ����������
    float diffuseCo = std::max(.0f, bc_normal.normalize() * lightDir.normalize()) * 0.6;
    TGAColor diffuseColor = orange;

    //��Ļ�ռ�߹�
    Vec3f halfVec = ((screenEye - bc_screen).normalize() + (screenLightDir - bc_screen).normalize()).normalize();
    Matrix mvp = GetMVP();
    Vec3f screenBcNormal = m2v(mvp * v2m(bc_normal));                      //������Ļ�ռ䷨��
    float similarCo = std::max(.0f, halfVec * screenBcNormal.normalize()); //��������뷨�ߵĽӽ��̶�
    float specularCo = pow(similarCo, 500);                                //�߹��������

    oColor = TGAColor(min(diffuseCo * diffuseColor.r + specularCo * 255, 255.f),
                      min(diffuseCo * diffuseColor.g + specularCo * 255, 255.f),
                      min(diffuseCo * diffuseColor.b + specularCo * 255, 255.f),
                      255);
    return false;
}
//----------------------------------------------------------------------------

// shadowmap����
//----------------------------------------------------------------------------
Vec3f ShadowMapShader::vertex(Model *model, int faceId, int i)
{
    std::vector<int> face = model->face(faceId);
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix sMVP = GetShadowMVP();

    Vec3f temp = m2v(sMVP * v2m(v));
    varying_z[i] = temp.z;
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������
}

bool ShadowMapShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    float intensity = min(bc_screen * varying_z, 1.f);
    oColor = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
    return false;
}
//----------------------------------------------------------------------------