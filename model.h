#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

//只需要提供路径，就可以直接反序列化一个带有两个容器的model，顶点序列和面序列
class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
};

#endif //__MODEL_H__