#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "tgaimage.h"
#include "geometry.h"
#include "pipeline.h"
#include "model.h"

class Shader
{

public:
    //可以看到，顶点着色器虽然叫着色器，但是其最主要的功能是
    //  1.将顶点坐标转换到屏幕空间
    //  2.为片元着色器的真正着色准备好数据，比如法线的变换，uv的变换，顶点高度的偏移
    //    所以不同的shader可以定义自己不同的希望在着色器之间传递的数据结构
    virtual Vec3f vertex(Model *model, int faceId, int i) = 0;
    //片元着色器才是真正决定像素颜色的地方
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