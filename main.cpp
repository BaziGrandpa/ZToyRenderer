#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "rasterization.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(51, 204, 51, 255);
const TGAColor pink = TGAColor(255, 153, 255, 255);
const TGAColor orange = TGAColor(255, 153, 102, 255);
const TGAColor blue = TGAColor(0, 102, 255, 255);

//定义光源方向
Vec3f lightDir = Vec3f(0, 1, 0);

Model *model = NULL;
const int width = 800;
const int height = 800;

int main(int argc, char **argv)
{
        //读模型
        model = new Model("obj/african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);

        for (int i = 0; i < model->nfaces(); i++)
        {
                std::vector<int> face = model->face(i);

                //转换屏幕坐标
                Vec2i screen_cor[3];
                Vec3f world_cors[3];
                for (int j = 0; j < 3; j++)
                {
                        Vec3f v = model->vert(face[j]);
                        screen_cor[j] = Vec2i((v.x + 1.) * width / 2, (v.y + 1.) * height / 2);
                        world_cors[j] = v;
                }
                //在世界坐标算光照和法线
                //先计算法线
                Vec3f normal = (world_cors[2] - world_cors[0]) ^ (world_cors[1] - world_cors[0]);
                //计算夹角
                normal.normalize();
                float intensity = normal * lightDir;
                //在屏幕空间着色
                if (intensity > 0)
                        RasterizedTiangle3(screen_cor, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
        return 0;
}
