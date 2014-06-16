#ifndef _CLOTHSIMULATOR_PLUGIN_H
#define _CLOTHSIMULATOR_PLUGIN_H

#include "clothSimulatorGlobal.h"
#include "Solver.hpp"
#include "subdivision.hpp"

extern "C" {

	_DLLExport bool initHashMesh(int vertexCount, glm::vec3* vertices, glm::vec3* normals, 
								 int triangleCount, glm::ivec3* triangles,
								 float scale);

	_DLLExport void updateMeshSpatial(glm::vec3* vertices, glm::vec3* normals, glm::ivec3* triangles);

	_DLLExport void clearMesh();

	_DLLExport bool initCloth(int vertexCount, glm::vec3* vertices, int triangleCount, glm::ivec3* triangles,
							  float edgeKs, float edgeKd, float bendKs, float bendKd);

	_DLLExport void getOuterTriangles(glm::ivec3* dstTriangles);

	_DLLExport void simulateCloth(float deltaT);

	_DLLExport void clearCloth();

	// Subdivide Mesh
	_DLLExport void subdivideMesh (int vertexCount, glm::vec3* vertices, 
								   int triangleCount, glm::ivec3* triangles, 
								   int subCount);
	_DLLExport int getSubdivisionVertexCnt();
	_DLLExport int getSubdivisionTriangleCnt();
	_DLLExport void getSubdivisionMesh(glm::vec3* vertices, glm::ivec3* triangles);
	_DLLExport void clearSubdivisionMesh();
};

#endif

