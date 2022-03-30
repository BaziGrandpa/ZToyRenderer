#include "head/tgaimage.h"
#include "head/model.h"
#include "head/geometry.h"
#include "head/rasterization.h"
#include "head/pipeline.h"
#include <iostream>
#include <cmath>
#include <algorithm>

int main(int argc, char **argv)
{
        //������������
        int width = 800;
        int height = 800;
        Vec3f lightDir = Vec3f(0, 0, 1);
        Vec3f eye(10, -2, 10);
        Vec3f center(0, 0, 0);
        LoadData("obj/african_head.obj", "texture/african_head_diffuse.tga");
        //��ʼ����������
        InitPipeline(width, height, lightDir);
        //��ʼ��mvp����
        InitMatrix(eye, center);
        //��Ⱦ
        Render();
        return 0;
}
