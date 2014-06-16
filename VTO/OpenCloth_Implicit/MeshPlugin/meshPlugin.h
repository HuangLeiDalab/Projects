#ifndef _MESH_PLUGIN_H
#define _MESH_PLUGIN_H

#include "utils.h"
#include "mesh.hpp"

extern "C" {

	_DLLExport bool initMesh(int vertexCount, glm::vec3* vertices, int triangleCount, glm::ivec3* triangles);

	_DLLExport void updateMeshSpatial();

	_DLLExport void collideMesh(glm::vec3* pre, glm::vec3* curr, glm::vec3* vel);

	_DLLExport void clearMesh();
};

#endif