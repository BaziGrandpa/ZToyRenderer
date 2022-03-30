#include "../head/geometry.h"
#include "../head/pipeline.h"
#include "../head/rasterization.h"
#include "../head/model.h"
#include <algorithm>
#include <cmath>
#include <iostream>

const int depth = 255; //����shadow map��ʱ��պ���Ϊ�Ҷ�
float *zbuffer;        //�����е�zbuffer
int width = 0;
int height = 0;

Model *model = NULL;
TGAImage *diffuse = NULL;
Matrix mvp;
Vec3f lightDir;

//����ģ��
void LoadData(const char *objName, const char *diffuseName)
{

    TGAImage a(100, 100, TGAImage::RGB);
    //��ģ
    model = new Model(objName);

    //����ͼ
    diffuse = new TGAImage();
    diffuse->read_tga_file(diffuseName);
}

//��ʼ������
// 1.��ʼ������ֱ���
// 2.zbuffer
void InitPipeline(int w, int h, Vec3f ld)
{
    width = w;
    height = h;
    zbuffer = new float[width * height];
    for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max())
        ;
    lightDir = ld;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize(); //����up��z����������Ҳ���������������������
    Vec3f y = cross(z, x).normalize();
    Matrix Minv = Matrix::identity();
    Matrix Tr = Matrix::identity();
    for (int i = 0; i < 3; i++)
    {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -eye[i];
    }
    return Minv * Tr;
}

Matrix ViewPort(int width, int height)
{
    Matrix viewport = Matrix::identity();
    viewport[0][0] = width / 2;
    viewport[0][3] = width / 2;
    viewport[1][1] = height / 2;
    viewport[1][3] = height / 2;
    // viewport[2][2] = depth / 2;
    // viewport[2][3] = depth / 2;
    viewport[2][2] = 1;
    viewport[2][3] = 0;
    return viewport;
}
//��ʼ�����б任����
// 1.Model�任��ģ������Ҫ������ת�����Ȱ�ģ�����ĵ��Ƶ���������ԭ�㣬����任���ƻ�ȥ��ֻ����ֵ��������������裬����������������Ⱦ
// 2.View�任��������ģ�ͱ任��������ռ䣬�����������������Ϊԭ�㣬һ��lookat��up��������һ����˷���
//      �������ȡ������һ����������Ϊֻ��Ⱦһ��ģ�ͣ����Կ��Զ���һ�����ĵ㣬
// 3.Project�任
//      ͶӰ����ƽ�棬��frustum���cube����Ҫ����Զ��ƽ����о������
//      Ҳ���Լ򵥵�ȫ��ͶӰ��Զƽ�棬����ʵ�ֵ�ͶӰ��û��fov�ĸ�����Ծ�����Զ�����Ҳ���Ǹı�һ��͸�ӱ����������������Ե�Զ��ĸо�
// 4.ViewPort�任�����д�[-1,1]*3�任����Ļ�ռ�
//      ���߳���(�ֱ���/2)����Ų����Ļ���ľͿ���
void InitMatrix(Vec3f eye, Vec3f center)
{
    Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
    Matrix Projection = Matrix::identity(); //�Ǽ򵥵�Զƽ��ͶӰ���������ƣ������������project
    Projection[3][2] = -1.f / (eye - center).norm();
    Matrix vp = ViewPort(width, height);

    std::cerr << ModelView << std::endl;
    std::cerr << Projection << std::endl;

    Matrix z = (vp * Projection * ModelView);
    std::cerr << z << std::endl;
    mvp = z;
}

//����ת���������շŵ�geometry��
Vec3f m2v(Matrix m)
{
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v)
{
    Matrix m = Matrix();
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

//���׼��������ʼ��Ⱦ
void Render()
{
    if (model == NULL)
    {
        std::cout << "load model failed.";
    }
    TGAImage output(width, height, TGAImage::RGB);
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f pts[3];
        Vec3f worldPos[3]; // objԭʼ��������
        Vec3f normals[3];
        Vec3f uvs[3];
        for (int i = 0; i < 3; i++) //һ�������������
        {
            int vertexId = i * 3;
            Vec3f v = model->vert(face[vertexId]);
            Vec3f temp = m2v(mvp * v2m(v));
            pts[i] = Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������

            int uvId = i * 3 + 1;
            Vec3f uv = model->uv(face[uvId]);
            uvs[i] = uv;

            int normalId = i * 3 + 2;
            Vec3f normal = model->normal(face[normalId]);
            normals[i] = normal;
        }
        RasterizedTiangle4(pts, normals, uvs, zbuffer, output, *diffuse, lightDir, false);
    }
    delete model;
    output.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    output.write_tga_file("output.tga");
}