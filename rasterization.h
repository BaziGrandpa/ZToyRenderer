#ifndef __RATERIZATION_H__
#define __RATERIZATION_H__

#include "geometry.h"
#include "tgaimage.h"
void RasterizedTiangle3(Vec3f *pts, TGAImage &image, TGAColor color);
void RasterizedTiangle4(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color);
#endif