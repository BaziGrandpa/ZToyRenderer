using namespace std;
#include "../head/tgaimage.h"
#include "../head/model.h"
#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/shader.h"
#include <cmath>
#include <algorithm>

//在顶点完成着色，fragment只是简单的插值
//----------------------------------------------------------------------------
Vec3f GouraudShader::vertex(Model *model, int faceId, int i)
{
    //拿到的是一个面的顶点坐标Id，uvId，normalId
    std::vector<int> face = model->face(faceId);

    //读世界法线
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    varing_intensity[i] = max(.0f, normal.normalize() * lightDir.normalize());

    //读世界坐标，然后做mvp变换
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();
    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题
}

//所有片元颜色一样
bool GouraudShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    float bc_intensity = min(varing_intensity * Vec3f(0.33, 0.33, 0.33), 1.f);
    oColor = TGAColor(white.r * bc_intensity, white.g * bc_intensity, white.b * bc_intensity, 255);
    return false;
}
//----------------------------------------------------------------------------

//基本的逐像素着色shader
//----------------------------------------------------------------------------
//一个面会调用这个函数三次，即三个点分别读取，i代表三角型的第几个顶点
Vec3f PhongShader::vertex(Model *model, int faceId, int i)
{
    //拿到的是一个面的顶点坐标Id，uvId，normalId
    std::vector<int> face = model->face(faceId);

    //根据索引读取uv
    int uvId = i * 3 + 1;
    Vec3f uv = model->uv(face[uvId]);
    uvs[i] = uv;

    //读世界法线
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    normals[i] = normal;

    //读世界坐标，然后做mvp变换
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();
    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题
}

//输入有一个重心坐标就行，引用为该片元的颜色
//返回值为是否渲染该片元
bool PhongShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{

    Vec3f bc_normal = Vec3f(0, 0, 0);
    Vec3f bc_uv = Vec3f(0, 0, 0);
    //用屏幕空间的重心对世界坐标z插值
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
        bc_uv = bc_uv + uvs[i] * bc_screen[i];
    }
    //照不到的就黑色，
    //这个光照之所以可以正确，因为这全是在世界坐标下的计算，相当于把模型一直摆在世界原点去做光照
    //更通用的做法，应该是把灯变换到切线空间，读normalmap的数据算
    float intensity = std::max(bc_normal.normalize() * lightDir.normalize(), 0.f);
    intensity = min(intensity + 0.1f, 1.f);
    TGAImage *diffuse = GetDiffuse();
    if (diffuse == nullptr) //没有漫反射则强行设false
        needDiffuse = false;

    TGAColor color = needDiffuse ? diffuse->get(bc_uv.x * diffuse->get_width(), (1 - bc_uv.y) * diffuse->get_height()) : white;
    oColor.r = intensity * color.r;
    oColor.g = intensity * color.g;
    oColor.b = intensity * color.b;
    oColor.a = 255;
    return false;
}
//----------------------------------------------------------------------------

//小卡通shader
//----------------------------------------------------------------------------
Vec3f ToonShader::vertex(Model *model, int faceId, int i)
{
    //拿到的是一个面的顶点坐标Id，uvId，normalId
    std::vector<int> face = model->face(faceId);

    //读世界法线
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    normals[i] = normal;

    //读世界坐标，然后做mvp变换
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();
    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题
}

bool ToonShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    Vec3f bc_normal = Vec3f(0, 0, 0);
    //用屏幕空间的重心对世界坐标z插值
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
    }
    //光看不到就全黑
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

//简单的支持高光shader
//----------------------------------------------------------------------------
BlinnPhongShader::BlinnPhongShader()
{
    //将观察向量与光照转换到屏幕空间
    Matrix mvp = GetMVP();
    screenEye = m2v(mvp * v2m(eye));
    screenLightDir = m2v(mvp * v2m(lightDir));
    InverseMVP = mvp.invert_transpose();
    cout << "eye:" << screenEye << "ld:" << screenLightDir;
}
Vec3f BlinnPhongShader::vertex(Model *model, int faceId, int i)
{
    //拿到的是一个面的顶点坐标Id，uvId，normalId
    std::vector<int> face = model->face(faceId);

    //根据索引读取uv
    int uvId = i * 3 + 1;
    Vec3f uv = model->uv(face[uvId]);
    uvs[i] = uv;

    //读世界法线
    int normalId = i * 3 + 2;
    Vec3f normal = model->normal(face[normalId]);
    normals[i] = normal;

    //读世界坐标，然后做mvp变换
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix mvp = GetMVP();

    Vec3f temp = m2v(mvp * v2m(v));
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题
}

//先不采样高光贴图了，直接当整个模型都有高光
bool BlinnPhongShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    Vec3f bc_normal = Vec3f(0, 0, 0);
    Vec3f bc_uv = Vec3f(0, 0, 0);
    //用屏幕空间的重心对世界坐标z插值
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
        bc_uv = bc_uv + uvs[i] * bc_screen[i];
    }
    //采样漫反射颜色
    TGAImage *diffuse = GetDiffuse();
    if (diffuse == nullptr)
        return true;
    int dWidth = diffuse->get_width();
    int dHeight = diffuse->get_height();
    TGAColor color = diffuse->get(bc_uv.x * dWidth, (1 - bc_uv.y) * dHeight);

    //在世界空间计算漫反射
    float diffuseCo = std::max(.0f, bc_normal.normalize() * lightDir.normalize()) * 0.6;
    TGAColor diffuseColor = orange;

    //屏幕空间高光
    Vec3f halfVec = ((screenEye - bc_screen).normalize() + (screenLightDir - bc_screen).normalize()).normalize();
    Matrix mvp = GetMVP();
    Vec3f screenBcNormal = m2v(mvp * v2m(bc_normal));                      //计算屏幕空间法线
    float similarCo = std::max(.0f, halfVec * screenBcNormal.normalize()); //半程向量与法线的接近程度
    float specularCo = pow(similarCo, 500);                                //高光区域控制

    oColor = TGAColor(min(diffuseCo * diffuseColor.r + specularCo * 255, 255.f),
                      min(diffuseCo * diffuseColor.g + specularCo * 255, 255.f),
                      min(diffuseCo * diffuseColor.b + specularCo * 255, 255.f),
                      255);
    return false;
}
//----------------------------------------------------------------------------

// shadowmap生成
//----------------------------------------------------------------------------
Vec3f ShadowMapShader::vertex(Model *model, int faceId, int i)
{
    std::vector<int> face = model->face(faceId);
    int vertexId = i * 3;
    Vec3f v = model->vert(face[vertexId]);
    Matrix sMVP = GetShadowMVP();

    Vec3f temp = m2v(sMVP * v2m(v));
    varying_z[i] = temp.z;
    return Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题
}

bool ShadowMapShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{
    float intensity = min(bc_screen * varying_z, 1.f);
    oColor = TGAColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
    return false;
}
//----------------------------------------------------------------------------