#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
        //首先保障x1>x2
        if (x1 < x0)
        {
                std::swap(x0, x1);
                std::swap(y0, y1);
        }

        bool isSteep = false;
        // y轴更长，对y轴采样
        if (y1 - y0 > x1 - x0)
        {
                std::swap(x0, y0);
                std::swap(x1, y1);
                isSteep = true;
        }
        //建立表达式，哪个轴长就对哪个轴采样
        float k = (float)(y1 - y0) / (x1 - x0);
        float b = y1 - k * x1;
        int xLength = x1 - x0;
        // y = k*x +b
        for (int i = 0; i <= xLength; i++)
        {
                /* code */
                int x = (x0 + i);
                int y = k * x + b;
                //只能set int 所以采样精度1已经足够了
                if (isSteep)
                        image.set(y, x, color);
                else
                        image.set(x, y, color);
        }
}

int main(int argc, char **argv)
{
        //读模型
        model = new Model("obj/african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);
        //根据网格画线
        for (int i = 0; i < model->nfaces(); i++)
        {
                //当前面
                std::vector<int> face = model->face(i);
                //取三个顶点
                for (int j = 0; j < 3; j++)
                {
                        Vec3f v0 = model->vert(face[j]);
                        Vec3f v1 = model->vert(face[(j + 1) % 3]);
                        // int x0 = v0.x;
                        // int y0 = v0.y;
                        // int x1 = v1.x;
                        // int y1 = v1.y;
                        int x0 = (v0.x + 1.) * 200. + 100.;//简单的变换，因为obj中的坐标都是归一化的
                        int y0 = (v0.y + 1.) * 200.;
                        int x1 = (v1.x + 1.) * 200. + 100.;
                        int y1 = (v1.y + 1.) * 200.;
                        line(x0, y0, x1, y1, image, white);
                }
        }
        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
        return 0;
}
