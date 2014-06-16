#ifndef _MESH_H_
#define _MESH_H_

#include <vector>
#include <glm/glm.hpp>

#define MIN_VEC3(a, b, m) \
	m[0] = a[0] < b[0] ? a[0] : b[0]; \
	m[1] = a[1] < b[1] ? a[1] : b[1]; \
	m[2] = a[2] < b[2] ? a[2] : b[2];

#define MAX_VEC3(a, b, m) \
	m[0] = a[0] > b[0] ? a[0] : b[0]; \
	m[1] = a[1] > b[1] ? a[1] : b[1]; \
	m[2] = a[2] > b[2] ? a[2] : b[2];

#define HASH_TABLE_SIZE 299


bool collideTriangle( glm::vec3* pos, glm::vec3* vel, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, float h);

struct Mesh {

	Mesh (int _vertexCount, glm::vec3* _vertices, int _triangleCount, glm::ivec3* _triangles){
		vertexCount = _vertexCount;
		vertices = _vertices;
		triangleCount = _triangleCount;
		triangles = _triangles;
	}

	int vertexCount;
	int triangleCount;
	glm::vec3* vertices;
	glm::ivec3* triangles;
};

struct HashSpatial {
	typedef struct _hashNode_ {
			std::vector<int> indices;
			glm::vec3 bboxMin;
			glm::vec3 bboxMax;
	} hashNode;

	typedef std::vector<int> hashIndice;

	HashSpatial (Mesh* _mesh) 
	{
		mMesh = _mesh;
		mTable.resize(HASH_TABLE_SIZE);
		hashIndice indiceArr;
		for (int i = 0; i < HASH_TABLE_SIZE; ++i){
			mTable[i] = indiceArr;
		}
		buildHashTable();
	}

	void updateHashTable()
	{
		clearHashTable();
		buildHashTable();
	}

	void intersect(glm::vec3* pre, glm::vec3* curr, glm::vec3* vel)
	{
		int cellID = intersect(curr);
		for (int n = 0; n < mTable[cellID].size(); ++n) {
			int nodeID = mTable[cellID][n];
			if (curr->x < mNodes[nodeID].bboxMin.x || curr->y < mNodes[nodeID].bboxMin.y || curr->z < mNodes[nodeID].bboxMin.z ||
				curr->x > mNodes[nodeID].bboxMax.x || curr->y > mNodes[nodeID].bboxMax.y || curr->z > mNodes[nodeID].bboxMax.z) {
					continue;
			}else{
				/*if (collider::collideTriangle(
				pre, curr, vel,
				mMesh->mVertices[mNodes[nodeID].indices[0]], 
				mMesh->mVertices[mNodes[nodeID].indices[1]],
				mMesh->mVertices[mNodes[nodeID].indices[2]]))
				return;*/
				if (collideTriangle(curr, vel, 
					mMesh->vertices[mNodes[nodeID].indices[0]],
					mMesh->vertices[mNodes[nodeID].indices[1]],
					mMesh->vertices[mNodes[nodeID].indices[2]],
					0.001f))
					return;
			}
		}
	}

	Mesh* mMesh;
	std::vector<hashNode> mNodes;
	std::vector<hashIndice> mTable;
	glm::vec3 mBBoxMin;
	glm::vec3 mBBoxMax;
	glm::vec3 mCellSize;

private:
	void buildHashTable()
	{
		mBBoxMin = mMesh->vertices[0];
		mBBoxMax = mMesh->vertices[0];
		mCellSize = glm::vec3(0, 0, 0);
		mNodes.resize(mMesh->triangleCount);
		for (int fn = 0; fn < mMesh->triangleCount; ++fn){
			glm::ivec3 indices = mMesh->triangles[fn];

			/* New hashNodes which are loaded into a List*/
			hashNode node;
			node.indices.push_back(indices.x);
			node.indices.push_back(indices.y);
			node.indices.push_back(indices.z);

			node.bboxMin = mMesh->vertices[indices.x];
			node.bboxMax = mMesh->vertices[indices.x];
			MIN_VEC3(node.bboxMin, mMesh->vertices[indices.y], node.bboxMin);
			MIN_VEC3(node.bboxMin, mMesh->vertices[indices.z], node.bboxMin);
			MAX_VEC3(node.bboxMax, mMesh->vertices[indices.y], node.bboxMax);
			MAX_VEC3(node.bboxMax, mMesh->vertices[indices.z], node.bboxMax);

			MIN_VEC3(node.bboxMin, mBBoxMin, mBBoxMin);
			MAX_VEC3(node.bboxMax, mBBoxMax, mBBoxMax);

			mNodes[fn] = node;
			/* Calculate the hash cell size */
			mCellSize += (node.bboxMax - node.bboxMin);
		}
		mCellSize = mCellSize * (1.0f / mMesh->triangleCount);

		int coord[6];
		int key;
		for (int n = 0; n < mNodes.size(); ++n){
			coord[0] = (mNodes[n].bboxMin.x - mBBoxMin.x) / mCellSize.x;
			coord[1] = (mNodes[n].bboxMin.y - mBBoxMin.y) / mCellSize.y;
			coord[2] = (mNodes[n].bboxMin.z - mBBoxMin.z) / mCellSize.z;
			coord[3] = (mNodes[n].bboxMax.x - mBBoxMin.x) / mCellSize.x;
			coord[4] = (mNodes[n].bboxMax.y - mBBoxMin.y) / mCellSize.y;
			coord[5] = (mNodes[n].bboxMax.z - mBBoxMin.z) / mCellSize.z;

			unsigned int cnt = 0;
			for (int i = coord[0]; i <= coord[3]; i++) {
				for (int j = coord[1]; j <= coord[4]; j++){
					for (int k = coord[2]; k <= coord[5]; k++){
						key = hashFunction(i, j, k);
						mTable[key].push_back(n);
					}
				}
			}
		}
	}

	void clearHashTable()
	{
		for (int i = 0; i < HASH_TABLE_SIZE; ++i){
			mTable[i].clear();
		}
	}

	int hashFunction(int x, int y, int z)
	{
		unsigned long hash = x * 73856093;
		hash = hash ^ (y * 19349663);
		hash = hash ^ (z * 83492791);
		return	hash % HASH_TABLE_SIZE;
	}

	int intersect(const glm::vec3* p)
	{
		if (p->x < mBBoxMin.x || p->y < mBBoxMin.y || p->z < mBBoxMin.z ||
			p->x > mBBoxMax.x || p->y > mBBoxMax.y || p->z > mBBoxMax.z) {
				return NULL;
		}
		int coord[3];
		coord[0] = (p->x - mBBoxMin.x) / mCellSize.x;
		coord[1] = (p->y - mBBoxMin.y) / mCellSize.y;
		coord[2] = (p->z - mBBoxMin.z) / mCellSize.z;
		return hashFunction(coord[0], coord[1], coord[2]);
	}
};


bool collideTriangle( glm::vec3* pos, glm::vec3* vel, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, float h)
{
	glm::vec3 t0t1 = t1 - t0;
	glm::vec3 t0t2 = t2 - t0;
	glm::vec3 t0P = *pos - t0;
	glm::vec3 n = glm::normalize(glm::cross(t0t1, t0t2));
	// The distance from P to the Triangle
	float d = glm::dot(t0P, n);
	// Check "pos - t * v" intersect with the Triangle
	float t = d / glm::dot(*vel, n);
	glm::vec3 newPos = *pos - t * (*vel);
	glm::vec3 p = newPos - t0;
	float dot00 = glm::dot(t0t2, t0t2); //e0.ddot(e0);
	float dot01 = glm::dot(t0t1, t0t2);//e0.ddot(e1);
	float dot02 = glm::dot(t0t2, p);//e0.ddot(e2);
	float dot11 = glm::dot(t0t1, t0t1);//e1.ddot(e1);
	float dot12 = glm::dot(t0t1, p);//e1.ddot(e2);
	float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);
	float _u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (_u < 0 || _u > 1)
	{
		return false;
	}
	float _v = (dot00 * dot12 - dot01 *dot02) * inverDeno;
	if (_v < 0 || _v > 1 || _u + _v > 1)
	{
		return false;
	}
	if (d < h)
	{
		*pos = *pos + n * (h - d);
	}
	// compute normal and tangential components of velocity
	//vel = 0.5f * vel;
	float tmpV = glm::dot(*vel, n);
	glm::vec3 vn = tmpV * n;
	glm::vec3 vt = *vel - vn;

	// new velocity = (1 - u)vt - e vn
	*vel = 0.004f * vt - 0.1f * vn;
	//vel = (0.05f) * vt - 0.8f * vn;
	//vel = glm::vec3(0.0f, 0.0f, 0.f);
	return true;
}
#endif
