#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "tgaimage.h"
#include "geometry.h"
#include "pipeline.h"
#include "model.h"

class Shader
{

public:
    virtual Vec3f vertex(Model *model, int faceId, int i) = 0;
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