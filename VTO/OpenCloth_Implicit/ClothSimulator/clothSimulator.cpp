#include "clothSimulator.h"

HashSpatial* spatial = NULL;
Cloth* cloth = NULL;
Subdivide* isoMesh = NULL;

_DLLExport bool initHashMesh( int vertexCount, glm::vec3* vertices, glm::vec3* normals, int triangleCount, glm::ivec3* triangles, float scale)
{
	clearMesh();
	spatial = new HashSpatial(vertexCount, vertices, normals, triangleCount, triangles, scale);
	return true;
}

_DLLExport void clearMesh()
{
	if (spatial != NULL){
		delete spatial;
		spatial = NULL;
	}
}

_DLLExport void updateMeshSpatial( glm::vec3* vertices, glm::vec3* normals, glm::ivec3* triangles )
{
	if (spatial != NULL){
		spatial->updateHashTable(vertices, normals, triangles);
	}
}


_DLLExport bool initCloth( int vertexCount, glm::vec3* vertices, int triangleCount, glm::ivec3* triangles, float edgeKs, float edgeKd, float bendKs, float bendKd )
{
	clearCloth();
	cloth = new Cloth(vertexCount, vertices, triangleCount, triangles, edgeKs, edgeKd, bendKs, bendKd);
	initalParams(spatial, cloth);
	return true;
}


_DLLExport void getOuterTriangles( glm::ivec3* dstTriangles )
{
	if (cloth != NULL){
		cloth->copySharedEdgeVertexIndex(dstTriangles);
	}
}


_DLLExport void clearCloth()
{
	clearParams();
	cloth = NULL;
}

_DLLExport void simulateCloth( float deltaT )
{
	stepPhysics(deltaT);
}

_DLLExport void subdivideMesh( int vertexCount, glm::vec3* vertices, int triangleCount, glm::ivec3* triangles, int subCount )
{
	if (isoMesh != NULL){
		clearSubdivisionMesh();
	}
	isoMesh = new Subdivide(vertexCount, vertices, triangleCount, triangles, subCount);
}

_DLLExport int getSubdivisionVertexCnt()
{
	if (isoMesh != NULL){
		return isoMesh->getVertexCnt();
	}
	return 0;
}


_DLLExport int getSubdivisionTriangleCnt()
{
	if (isoMesh != NULL){
		return isoMesh->getTriangleCnt();
	}
	return 0;
}


_DLLExport void getSubdivisionMesh( glm::vec3* vertices, glm::ivec3* triangles )
{
	if (isoMesh != NULL){
		isoMesh->copyMesh(vertices, triangles);
	}
}

_DLLExport void clearSubdivisionMesh()
{
	if (isoMesh != NULL){
		delete isoMesh;
		isoMesh = NULL;
	}
}
