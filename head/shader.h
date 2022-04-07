#ifndef __SHADER_H__
#define __SHADER_H__

#include "geometry.h"
#include "pipeline.h"

extern int width;
extern int height;
extern Vec3f lightDir;
extern Vec3f eye;
extern Vec3f center;

//逐顶点
class GouraudShader : public Shader
{
    //保存顶点计算的光照强度
    Vec3f varing_intensity;

public:
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};
// Phongshading，是逐像素的
class PhongShader : public Shader
{
private:
    //这里不用数组是因为，用一个三维向量存三个值
    Vec3f intensity;

    //以下两个变量要存三个三维变量
    Vec3f normals[3];
    Vec3f uvs[3];
    //控制是否需要漫反射
    bool needDiffuse = true;

public:
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};

class ToonShader : public Shader
{
    //传递一下法线就可以了
    Vec3f normals[3];

public:
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};

class BlinnPhongShader : public Shader
{

    Vec3f normals[3];
    Vec3f uvs[3];
    Vec3f screenLightDir;
    Vec3f screenEye;
    Matrix InverseMVP; //用于将法线从世界空间变换到屏幕空间

public:
    BlinnPhongShader();
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};

class ShadowMapShader : public Shader
{
private:
    Vec3f varying_z;

public:
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};
#endif