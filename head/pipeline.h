#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include "tgaimage.h"

void LoadData(const char *objName, const char *diffuseName);
void InitPipeline(int w, int h, Vec3f ld);
void InitMatrix(Vec3f eye, Vec3f center);
void Render();
Vec3f m2v(Matrix m);
Matrix v2m(Vec3f v);
#endif