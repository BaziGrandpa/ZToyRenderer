#ifndef __SHADER_H__
#define __SHADER_H__

#include "geometry.h"
#include "pipeline.h"

extern int width;
extern int height;
extern Vec3f lightDir;
extern Vec3f eye;
extern Vec3f center;

//�𶥵�
class GouraudShader : public Shader
{
    //���涥�����Ĺ���ǿ��
    Vec3f varing_intensity;

public:
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};
// Phongshading���������ص�
class PhongShader : public Shader
{
private:
    //���ﲻ����������Ϊ����һ����ά����������ֵ
    Vec3f intensity;

    //������������Ҫ��������ά����
    Vec3f normals[3];
    Vec3f uvs[3];
    //�����Ƿ���Ҫ������
    bool needDiffuse = true;

public:
    virtual Vec3f vertex(Model *model, int faceId, int i);
    virtual bool fragment(Vec3f bc_screen, TGAColor &oColor);
};

class ToonShader : public Shader
{
    //����һ�·��߾Ϳ�����
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
    Matrix InverseMVP; //���ڽ����ߴ�����ռ�任����Ļ�ռ�

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