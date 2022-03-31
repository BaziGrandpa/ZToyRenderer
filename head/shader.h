#ifndef __SHADER_H__
#define __SHADER_H__

#include "geometry.h"
#include "pipeline.h"

extern int width;
extern int height;
extern Vec3f lightDir;
extern Vec3f eye;
extern Vec3f center;

//���������������ɫ
class GouraudShader : public Shader
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

#endif