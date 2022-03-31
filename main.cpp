#include "head/pipeline.h"
#include "head/geometry.h"
#include "head/shader.h"

//�����������ã�shader���Է��ʵ�ȫ�ֱ������൱��ȫ�ֺ����
int width = 800;
int height = 800;
Vec3f lightDir = Vec3f(1, 1, 1);
Vec3f eye(0, 0.2, 1);
Vec3f center(0, 0, 0);

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
        // ToonShader shader;
        //��Ⱦ
        Render(&shader);
        return 0;
}
