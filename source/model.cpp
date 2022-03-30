#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "../head/model.h"

Model::Model(const char *filename) : verts_(), faces_()
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
        return;
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) //¶Á¶¥µã
        {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++)
                iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vt ")) //¶Áuv
        {
            iss >> trash >> trash;
            Vec3f uv;
            for (int i = 0; i < 3; i++)
                iss >> uv[i];
            tex_coord.push_back(uv);
        }
        else if (!line.compare(0, 3, "vn ")) //¶Áuv
        {
            iss >> trash >> trash;
            Vec3f uv;
            for (int i = 0; i < 3; i++)
                iss >> uv[i];
            normals_.push_back(uv);
        }
        else if (!line.compare(0, 2, "f "))
        {
            std::vector<int> f;
            int itrash, idx;
            iss >> trash;
            // while (iss >> idx >> trash >> itrash >> trash >> itrash)
            for (int i = 0; i < 3; i++)
            {
                iss >> idx;
                f.push_back(--idx);
                for (int j = 0; j < 2; j++)
                {
                    iss >> trash >> idx;
                    f.push_back(--idx);
                }
            }
            faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model()
{
}

int Model::nverts()
{
    return (int)verts_.size();
}

int Model::nfaces()
{
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx)
{
    return faces_[idx];
}

Vec3f Model::vert(int i)
{
    return verts_[i];
}

Vec3f Model::uv(int i)
{
    return tex_coord[i];
}

Vec3f Model::normal(int i)
{
    return normals_[i];
}
