#include "head/pipeline.h"
#include "head/geometry.h"
#include "head/shader.h"

//场景基本配置，shader可以访问的全局变量，相当于全局宏哈哈
int width = 800;
int height = 800;
Vec3f lightDir(2, 2, 0);
Vec3f eye(0, 0, 1);
Vec3f center(0, 0, 0);

int main(int argc, char **argv)
{
        //加载模型，贴图
        LoadData("obj/african_head.obj", "texture/african_head_diffuse.tga");
        // LoadData("obj/diablo3_pose.obj", "");
        //初始化管线设置
        InitPipeline(width, height);
        //初始化mvp矩阵
        InitMatrix(eye, lightDir, center);
        //选择你的shader
        GouraudShader shader;
        // PhongShader shader;
        //   ToonShader shader;
        //   BlinnPhongShader shader;
        //   ShadowMapShader shader;
        //渲染
        Render(&shader);
        return 0;
}
