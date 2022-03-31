#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "tgaimage.h"
#include "geometry.h"
#include "pipeline.h"
#include "model.h"

class Shader
{

public:
    //���Կ�����������ɫ����Ȼ����ɫ��������������Ҫ�Ĺ�����
    //  1.����������ת������Ļ�ռ�
    //  2.ΪƬԪ��ɫ����������ɫ׼�������ݣ����編�ߵı任��uv�ı任������߶ȵ�ƫ��
    //    ���Բ�ͬ��shader���Զ����Լ���ͬ��ϣ������ɫ��֮�䴫�ݵ����ݽṹ
    virtual Vec3f vertex(Model *model, int faceId, int i) = 0;
    //ƬԪ��ɫ��������������������ɫ�ĵط�
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void LoadData(const char *objName, const char *diffuseName);
void InitPipeline(int w, int h);
void InitMatrix(Vec3f eye, Vec3f center);
void Render(Shader *shader);
Vec3f m2v(Matrix m);
Matrix v2m(Vec3f v);
Matrix GetMVP();
TGAImage *GetDiffuse();

#endif