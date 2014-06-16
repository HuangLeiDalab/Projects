#ifndef AABBTREE_H
#define AABBTREE_H

#include "utils.h"
#include "mesh.h"

class Mesh;

class BBox
{
public:
	vec3 inf, sup;

	BBox(){}
	virtual void computeBounds(Mesh *) = 0;
	virtual bool intersect(BBox *bbox) = 0;
	virtual uvec3 getIndices() = 0;
	virtual ~BBox(){};

	void render();

protected:
	friend class AABBNode;
};

class AABBNode : public BBox
{
public:
	AABBNode *left;
	AABBNode *right;
	int vertexCount;
	int depth;

	AABBNode(){}

	virtual void computeBounds(Mesh *m);

	virtual bool intersect(BBox *bbox);

	virtual uvec3 getIndices();
};

class AABBLeaf : public AABBNode
{
private:
	int a, b, c;

public:
	AABBLeaf(int a_, int b_, int c_);

	virtual void computeBounds(Mesh *m);

	virtual uvec3 getIndices();
};

struct AABBTreeInfo {

	int maxTreeDepth;		
	int leafVertexCount;		  
};

#endif // AABBTREE_H
