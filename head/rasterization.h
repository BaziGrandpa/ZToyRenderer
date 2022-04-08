#ifndef __RATERIZATION_H__
#define __RATERIZATION_H__

#include "geometry.h"
#include "tgaimage.h"
#include "pipeline.h"

void RasterizedTiangle3(Vec3f *pts, TGAImage &image, TGAColor color);
void RasterizedTiangle4(Vec3f *pts, Vec3f *normals, Vec3f *uvs, float *zbuffer, TGAImage &image, TGAImage &diffuse, Vec3f &lightDir, bool needDiffuse);
void RasterizeWithShader(Shader *Shader, Vec3f *screenCord, float *zbuffer, TGAImage &image, bool ssaa);
#endif