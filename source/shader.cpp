using namespace std;
#include "../head/tgaimage.h"
#include "../head/model.h"
#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/shader.h"
#include <cmath>
#include <algorithm>

//基本的逐像素光照shader
//----------------------------------------------------------------------------
//一个面会调用这个函数三次，即三个点分别读取，i代表三角型的第几个顶点
Vec3f GouraudShader::vertex(Model *model, int faceId, int i)
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
bool GouraudShader::fragment(Vec3f bc_screen, TGAColor &oColor)
{

    Vec3f bc_normal = Vec3f(0, 0, 0);
    Vec3f bc_uv = Vec3f(0, 0, 0);
    //用屏幕空间的重心对世界坐标z插值
    for (int i = 0; i < 3; i++)
    {
        bc_normal = bc_normal + normals[i] * bc_screen[i];
        bc_uv = bc_uv + uvs[i] * bc_screen[i];
    }
    //假设正面001打光，照不到的就黑色
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
