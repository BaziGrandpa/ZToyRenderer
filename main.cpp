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
        //场景基本配置
        int width = 800;
        int height = 800;
        Vec3f lightDir = Vec3f(0, 0, 1);
        Vec3f eye(10, -2, 10);
        Vec3f center(0, 0, 0);
        LoadData("obj/african_head.obj", "texture/african_head_diffuse.tga");
        //初始化管线设置
        InitPipeline(width, height, lightDir);
        //初始化mvp矩阵
        InitMatrix(eye, center);
        //渲染
        Render();
        return 0;
}
