#include "head/tgaimage.h"
#include "head/model.h"
#include "head/geometry.h"
#include "head/rasterization.h"
#include <iostream>
#include <cmath>
#include <algorithm>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(51, 204, 51, 255);
const TGAColor pink = TGAColor(255, 153, 255, 255);
const TGAColor orange = TGAColor(255, 153, 102, 255);
const TGAColor blue = TGAColor(0, 102, 255, 255);

//�����Դ����
Vec3f lightDir = Vec3f(0, 0, -1);
Vec3f eye(10, 0, 10);
Vec3f center(0, 0, 0);

Model *model = NULL;
TGAImage *diffuse = NULL;
const int width = 800;
const int height = 800;
const int depth = 255; //����shadow map��ʱ��պ���Ϊ�Ҷ�

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

Matrix ViewPort()
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
// 1.Model�任��ģ������Ҫ������ת�����Ȱ�ģ�����ĵ��Ƶ���������ԭ�㣬����任���ƻ�ȥ��ֻ����ֵ��������������裬���������������Ⱦ
// 2.View�任��������ģ�ͱ任��������ռ䣬�����������������Ϊԭ�㣬һ��lookat��up��������һ����˷���
//      �������ȡ������һ����������Ϊֻ��Ⱦһ��ģ�ͣ����Կ��Զ���һ�����ĵ㣬
// 3.Project�任
//      ͶӰ����ƽ�棬��frustum���cube����Ҫ����Զ��ƽ����о������
//      Ҳ���Լ򵥵�ȫ��ͶӰ��Զƽ�棬����ʵ�ֵ�ͶӰ��û��fov�ĸ�����Ծ�����Զ�����Ҳ���Ǹı�һ��͸�ӱ����������������Ե�Զ��ĸо�
// 4.ViewPort�任�����д�[-1,1]*3�任����Ļ�ռ�
//      ��߳���(�ֱ���/2)����Ų����Ļ���ľͿ���
Matrix InitMatrix()
{
        Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
        Matrix Projection = Matrix::identity(); //�Ǽ򵥵�Զƽ��ͶӰ���������ƣ������������project
        Projection[3][2] = -1.f / (eye - center).norm();
        Matrix vp = ViewPort();

        std::cerr << ModelView << std::endl;
        std::cerr << Projection << std::endl;

        Matrix z = (vp * Projection * ModelView);
        std::cerr << z << std::endl;
        return z;
}

//����ת����
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

Vec3f world2screen(Vec3f v)
{
        // return Vec3f(int((v.x + 1.) * (width - 200) / 2. + 100.), int((v.y + 1.) * (height - 200) / 2. + 100.), v.z);
        return Vec3f(int((v.x * (width) / 2 + width / 2)), int((v.y * (height) / 2 + height / 2)), v.z);
}
int main(int argc, char **argv)
{
        //��ģ��
        model = new Model("obj/african_head.obj");

        //����ͼ
        diffuse = new TGAImage();
        diffuse->read_tga_file("texture/african_head_diffuse.tga");

        float *zbuffer = new float[width * height];
        for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max())
                ;

        TGAImage image(width, height, TGAImage::RGB);
        //��ȡmvp�任
        Matrix mvp = InitMatrix();

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
                        // pts[i] = world2screen(v);
                        Vec3f temp = m2v(mvp * v2m(v));
                        pts[i] = Vec3f((int)temp.x, (int)temp.y, temp.z); //��ת�����ڲ�ֵ����ʱ������

                        int uvId = i * 3 + 1;
                        Vec3f uv = model->uv(face[uvId]);
                        uvs[i] = uv;

                        int normalId = i * 3 + 2;
                        Vec3f normal = model->normal(face[normalId]);
                        normals[i] = normal;
                }

                RasterizedTiangle4(pts, normals, uvs, zbuffer, image, diffuse, true);
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
        return 0;
}
