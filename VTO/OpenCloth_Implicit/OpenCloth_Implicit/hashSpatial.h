#ifndef _HASH_SPATIAL_H
#define _HASH_SPATIAL_H

#include "mesh.h"

#define HASH_TABLE_SIZE 299

struct hashNode {
	std::vector<unsigned int> indices;
	vec3 bboxMin;
	vec3 bboxMax;
};

class hashSpatial
{
	typedef std::vector<USHORT> hashIndice;
public:
	hashSpatial(Mesh *mesh);
	~hashSpatial();

	void updateHashTable();
	void intersect(vec3& pre, vec3& curr, vec3& vel);

	// TO DELET, JUST FOR TEST
	void drawBBox();
	void drawBBox(const vec3& min, const vec3 max);

private:
	void buildHashTable();
	inline void clearHashTable();
	inline int hashFunction(const int x, const int y, const int z);
	int intersect(const vec3& p);

private:
	Mesh* mMesh;
	std::vector<hashNode> mNodes;
	std::vector<hashIndice> mTable;
	vec3 mBBoxMin;
	vec3 mBBoxMax;
	vec3 mCellSize;
};

class collider
{
public:
	static bool collideTriangle( vec3& prePos, vec3& currPos, vec3& velocity, const vec3 t0, const vec3 t1, const vec3 t2);
	static bool collideTriangle( vec3& pos, vec3& vel, const vec3 t0, const vec3 t1, const vec3 t2, const float h);
};
#endif