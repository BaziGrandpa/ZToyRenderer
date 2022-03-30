#include "head/tgaimage.h"
#include "head/model.h"
#include "head/geometry.h"
#include "head/pipeline.h"
#include <cmath>
#include <algorithm>

//场景基本配置，shader可以访问的全局变量，相当于全局宏哈哈
int width = 800;
int height = 800;
Vec3f lightDir = Vec3f(0, 0, 1);
Vec3f eye(10, -2, 10);
Vec3f center(0, 0, 0);
bool needDiffuse = false;

class GouraudShader : public Shader
{
        //这里不用数组是因为，用一个三维向量存三个值
        Vec3f intensity;

        //以下两个变量要存三个三维变量
        Vec3f normals[3];
        Vec3f uvs[3];

        //返回处理完后的屏幕坐标
        //一个面会调用这个函数三次，即三个点分别读取，i代表第几次
        virtual Vec3f vertex(Model *model, int faceId, int i)
        {
                //拿到的是一个面的顶点坐标Id，uvId，normalId
                std::vector<int> face = model->face(faceId);

                int uvId = i * 3 + 1;
                Vec3f uv = model->uv(face[uvId]);
                uvs[i] = uv;

                int normalId = i * 3 + 2;
                Vec3f normal = model->normal(face[normalId]);
                normals[i] = normal;

                int vertexId = i * 3;
                Vec3f v = model->vert(face[vertexId]);
                Matrix mvp = GetMVP();
                Vec3f temp = m2v(mvp * v2m(v));
                return Vec3f((int)temp.x, (int)temp.y, temp.z); //不转整型在插值计算时出问题
        }

        //输入有一个重心坐标就行，引用为该片元的颜色
        //返回值为是否渲染该片元
        virtual bool fragment(Vec3f bc_screen, TGAColor &oColor)
        {

                Vec3f bc_normal = Vec3f(0, 0, 0);
                Vec3f bc_uv = Vec3f(0, 0, 0);
                //用屏幕空间的重心对世界坐标z插值
                for (int i = 0; i < 3; i++)
                {
                        bc_normal = bc_normal + normals[i] * bc_screen[i];
                        bc_uv = bc_uv + uvs[i] * bc_screen[i];
                }
                //假设正面001打光，照不到的就黑色
                float intensity = std::max(bc_normal.normalize() * lightDir.normalize(), 0.f);
                TGAImage *diffuse = GetDiffuse();
                int dWidth = diffuse->get_width();
                int dHeight = diffuse->get_height();
                TGAColor color = needDiffuse ? diffuse->get(bc_uv.x * dWidth, (1 - bc_uv.y) * dHeight) : white;
                // image.set(P.x, P.y, TGAColor(intensity * color.r, intensity * color.g, intensity * color.b, 255));
                oColor.r = intensity * color.r;
                oColor.g = intensity * color.g;
                oColor.b = intensity * color.b;
                oColor.a = 255;
                return false;
        }
};

int main(int argc, char **argv)
{

        //加载模型，贴图
        LoadData("obj/african_head.obj", "texture/african_head_diffuse.tga");
        //初始化管线设置
        InitPipeline(width, height);
        //初始化mvp矩阵
        InitMatrix(eye, center);
        //选择你的shader
        GouraudShader shader;
        //渲染
        Render(&shader);
        return 0;
}
