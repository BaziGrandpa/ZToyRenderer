#include "head/tgaimage.h"
#include "head/model.h"
#include "head/geometry.h"
#include "head/pipeline.h"
#include <cmath>
#include <algorithm>

//�����������ã�shader���Է��ʵ�ȫ�ֱ������൱��ȫ�ֺ����
int width = 800;
int height = 800;
Vec3f lightDir = Vec3f(0, 0, 1);
Vec3f eye(10, -2, 10);
Vec3f center(0, 0, 0);
bool needDiffuse = false;

class GouraudShader : public Shader
{
        //���ﲻ����������Ϊ����һ����ά����������ֵ
        Vec3f intensity;

        //������������Ҫ��������ά����
        Vec3f normals[3];
        Vec3f uvs[3];

        //���ش���������Ļ����
        //һ������������������Σ���������ֱ��ȡ��i����ڼ���
        virtual Vec3f vertex(Model *model, int faceId, int i)
        {
                //�õ�����һ����Ķ�������Id��uvId��normalId
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
                return Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������
        }

        //������һ������������У�����Ϊ��ƬԪ����ɫ
        //����ֵΪ�Ƿ���Ⱦ��ƬԪ
        virtual bool fragment(Vec3f bc_screen, TGAColor &oColor)
        {

                Vec3f bc_normal = Vec3f(0, 0, 0);
                Vec3f bc_uv = Vec3f(0, 0, 0);
                //����Ļ�ռ�����Ķ���������z��ֵ
                for (int i = 0; i < 3; i++)
                {
                        bc_normal = bc_normal + normals[i] * bc_screen[i];
                        bc_uv = bc_uv + uvs[i] * bc_screen[i];
                }
                //��������001��⣬�ղ����ľͺ�ɫ
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

        //����ģ�ͣ���ͼ
        LoadData("obj/african_head.obj", "texture/african_head_diffuse.tga");
        //��ʼ����������
        InitPipeline(width, height);
        //��ʼ��mvp����
        InitMatrix(eye, center);
        //ѡ�����shader
        GouraudShader shader;
        //��Ⱦ
        Render(&shader);
        return 0;
}
