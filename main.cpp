#include "head/pipeline.h"
#include "head/geometry.h"
#include "head/shader.h"

//�����������ã�shader���Է��ʵ�ȫ�ֱ������൱��ȫ�ֺ����
int width = 800;
int height = 800;
Vec3f lightDir(2, 2, 0);
Vec3f eye(0, 0, 1);
Vec3f center(0, 0, 0);

int main(int argc, char **argv)
{
        //����ģ�ͣ���ͼ
        LoadData("obj/african_head.obj", "texture/african_head_diffuse.tga");
        // LoadData("obj/diablo3_pose.obj", "");
        //��ʼ����������
        InitPipeline(width, height);
        //��ʼ��mvp����
        InitMatrix(eye, lightDir, center);
        //ѡ�����shader
        GouraudShader shader;
        // PhongShader shader;
        //   ToonShader shader;
        //   BlinnPhongShader shader;
        //   ShadowMapShader shader;
        //��Ⱦ
        Render(&shader);
        return 0;
}
