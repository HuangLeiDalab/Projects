#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "mesh.h"

class ObjLoader
{
public:
	ObjLoader(){}
	~ObjLoader(){}

	static bool loadObj(const QString filename, Mesh *mesh);
	static bool loadMtl(const QString filename, Mesh *mesh);
};

#endif