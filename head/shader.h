#ifndef __SHADER_H__
#define __SHADER_H__

#include "geometry.h"
#include "pipeline.h"

extern int width;
extern int height;
extern Vec3f lightDir;
extern Vec3f eye;
extern Vec3f center;

//最基本的漫反射着色
class GouraudShader : public Shader
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

#endif