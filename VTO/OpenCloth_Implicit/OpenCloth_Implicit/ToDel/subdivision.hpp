#ifndef _SUBDIVIDE_H_
#define _SUBDIVIDE_H_

#include <vector>
#include <map>
#include <glm/glm.hpp>

static int subDivideMap2[] = { 0, 1, 5, 1, 2, 3, 1, 3, 5, 5, 3, 4 };
static int subDivideMap3[] = { 0, 1, 8, 1, 2, 9, 1, 9, 8, 8, 9, 7, 2, 3, 4, 2, 4, 9, 9, 4, 5, 9, 5, 7, 7, 5, 6 };
static int subDivideMap4[] = { 0, 1, 11, 1, 2, 12, 1, 12, 11, 11, 12, 10, 2, 3, 13, 2, 13, 12, 12, 13, 14, 12, 14, 10, 
						      10, 14, 9, 3, 4, 5, 3, 5, 13, 13, 5, 6, 13, 6, 14, 14, 6, 7, 14, 7, 9, 9, 7, 8 };
static int subDivideMap5[] = { 0, 1, 14, 1, 2, 15, 1, 15, 14, 14, 15, 13, 2, 3, 16, 2, 16, 15, 15, 16, 17, 15, 17, 13,
							  13, 17, 12, 3, 4, 18, 3, 18, 16, 16, 18, 19, 16, 19, 17, 17, 19, 20, 17, 20, 12, 12, 20, 
							  11, 4, 5, 6, 4, 6, 18, 18, 6, 7, 18, 7, 19, 19, 7, 8, 19, 8, 20, 20, 8, 9, 20, 9, 11, 11, 
							  9, 10};

static int* subDivideMap[6] = { NULL, NULL, &subDivideMap2[0], &subDivideMap3[0], &subDivideMap4[0], &subDivideMap5[0] };


struct SubdivideEdge{
	int e1, e2;
	bool isExchanged;
	SubdivideEdge(const int _e1, const int _e2){
		if (_e1 < _e2){
			e1 = _e1;
			e2 = _e2;
			isExchanged = false;
		}else{
			e1 = _e2;
			e2 = _e1;
			isExchanged = true;
		}
	}
	bool operator < (const SubdivideEdge& e) const {
		if (e1 == e.e1)
			return e2 < e.e2;
		else
			return e1 < e.e1;
	}
};

class Subdivide {

public:
	Subdivide (int _vertexCount, glm::vec3* _vertices, int _triangleCount, glm::ivec3* _triangles, int subCount);
	~Subdivide();

	bool copyMesh (glm::vec3* dstVertices, glm::ivec3* triangles);

	int getVertexCnt() const { return vertexCount; }
	int getTriangleCnt() const { return triangleCount; }
	glm::vec3* getVertices() { return &vertices[0]; }
	glm::ivec3* getTriangles() { return &triangles[0]; }

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> triangles;
	int vertexCount;
	int triangleCount;

};

Subdivide::Subdivide(int _vertexCount, glm::vec3* _vertices, int _triangleCount, glm::ivec3* _triangles, int subCount = 3)
	:vertexCount(_vertexCount), triangleCount(_triangleCount * subCount * subCount)
{
	vertices.resize(vertexCount);
	triangles.resize(triangleCount);
	for (int i = 0; i < vertexCount; ++i){
		vertices[i] = _vertices[i];
	}
	std::map<SubdivideEdge, std::vector<int> > edgeMap;
	int triIndex = 0;
	for (int i = 0; i < _triangleCount; ++i){
		std::vector<int> triIndices;
		// Edge 1
		SubdivideEdge edge1(_triangles[i].x, _triangles[i].y);
		if (edgeMap.find(edge1) == edgeMap.end()){
			glm::vec3 vertexGap = (vertices[edge1.e2] - vertices[edge1.e1]) / (float)subCount;
			for (int i = 1; i < subCount; ++i){
				vertices.push_back(vertices[edge1.e1] + vertexGap * (float)i);
				edgeMap[edge1].push_back(vertexCount++);
			}
		}
		if (edge1.isExchanged){
			triIndices.push_back(edge1.e2);
			const std::vector<int>& tmpI = edgeMap[edge1];
			for (int i = 0; i < tmpI.size(); ++i){
				triIndices.push_back(tmpI[tmpI.size() - i - 1]);
			}
		}else{
			triIndices.push_back(edge1.e1);
			const std::vector<int>& tmpI = edgeMap[edge1];
			for (int i = 0; i < tmpI.size(); ++i){
				triIndices.push_back(tmpI[i]);
			}
		}
		// Edge 2
		SubdivideEdge edge2(_triangles[i].y, _triangles[i].z);
		if (edgeMap.find(edge2) == edgeMap.end()){
			glm::vec3 vertexGap = (vertices[edge2.e2] - vertices[edge2.e1]) / (float)subCount;
			for (int i = 1; i < subCount; ++i){
				vertices.push_back(vertices[edge2.e1] + vertexGap * (float)i);
				edgeMap[edge2].push_back(vertexCount++);
			}
		}
		if (edge2.isExchanged){	
			triIndices.push_back(edge2.e2);
			const std::vector<int>& tmpI = edgeMap[edge2];
			for (int i = 0; i < tmpI.size(); ++i){
				triIndices.push_back(tmpI[tmpI.size() - i - 1]);
			}			
		}else{			
			triIndices.push_back(edge2.e1);
			const std::vector<int>& tmpI = edgeMap[edge2];
			for (int i = 0; i < tmpI.size(); ++i){
				triIndices.push_back(tmpI[i]);
			}
		}
		// Edge 3
		SubdivideEdge edge3(_triangles[i].z, _triangles[i].x);
		if (edgeMap.find(edge3) == edgeMap.end()){
			glm::vec3 vertexGap = (vertices[edge3.e2] - vertices[edge3.e1]) / (float)subCount;
			for (int i = 1; i < subCount; ++i){
				vertices.push_back(vertices[edge3.e1] + vertexGap * (float)i);
				edgeMap[edge3].push_back(vertexCount++);
			}
		}
		if (edge3.isExchanged){
			triIndices.push_back(edge3.e2);
			const std::vector<int>& tmpI = edgeMap[edge3];
			for (int i = 0; i < tmpI.size(); ++i){
				triIndices.push_back(tmpI[tmpI.size() - i - 1]);
			}
		}else{
			triIndices.push_back(edge3.e1);
			const std::vector<int>& tmpI = edgeMap[edge3];
			for (int i = 0; i < tmpI.size(); ++i){
				triIndices.push_back(tmpI[i]);
			}
		}

		// Limit the subCount to [2, 5]
		subCount = subCount < 2 ? 2 : subCount;
		subCount = subCount > 5 ? 5 : subCount;

		// Create vertices inner triangle
		int currTriIndicesCnt = triIndices.size();
		for (int step = 2; step < subCount; ++step){
			glm::vec3 vertexGap = (vertices[triIndices[currTriIndicesCnt - step]] - vertices[triIndices[step]]) / (float)step;
			for (int i = 1; i < step; ++i){
				vertices.push_back(vertices[triIndices[step]] + vertexGap * (float)i);
				triIndices.push_back(vertexCount++);
			}
		}

		// Create triangle index
		int size = subCount * subCount;
		for (int i = 0; i < size; i++){
			triangles[triIndex++] = glm::ivec3(triIndices[subDivideMap[subCount][i * 3]],
											   triIndices[subDivideMap[subCount][i * 3 + 1]],
										       triIndices[subDivideMap[subCount][i * 3 + 2]]);
		}
	}
}

Subdivide::~Subdivide()
{
	vertices.clear();
	triangles.clear();
}

bool Subdivide::copyMesh( glm::vec3* dstVertices, glm::ivec3* dstTriangles )
{
	if (dstVertices == NULL || dstTriangles == NULL){
		return false;
	}
	for (int i = 0; i < vertexCount; ++i){
		dstVertices[i] = vertices[i];
	}
	for (int i = 0; i < triangleCount; ++i){
		dstTriangles[i] = triangles[i];
	}
	return true;
}

#endif