#include "meshPlugin.h"

Mesh *mesh = NULL;
HashSpatial *spatial = NULL;


_DLLExport bool initMesh( int vertexCount, glm::vec3* vertices, int triangleCount, glm::ivec3* triangles )
{
	clearMesh();
	mesh = new Mesh(vertexCount, vertices, triangleCount, triangles);
	spatial = new HashSpatial(mesh);
	return true;
}

_DLLExport void clearMesh()
{
	if (mesh != NULL){
		delete mesh;
		mesh = NULL;
	}
	if (spatial != NULL){
		delete spatial;
		spatial = NULL;
	}
}

_DLLExport void updateMeshSpatial()
{
	if (spatial != NULL){
		spatial->updateHashTable();
	}
}

_DLLExport void collideMesh( glm::vec3* pre, glm::vec3* curr, glm::vec3* vel )
{
	if (spatial != NULL){
		spatial->intersect(pre, curr, vel);
	}
}
