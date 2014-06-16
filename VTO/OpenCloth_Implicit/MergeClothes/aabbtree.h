#ifndef AABBTREE_H
#define AABBTREE_H

#include "utils.h"

class Mesh;

class BBox
{
public:
	Vec3d inf, sup;

	BBox(){}
	virtual void computeBounds(Mesh *) = 0;
	virtual bool intersect(BBox *bbox) = 0;
	virtual Vec3i getIndices() = 0;
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

	virtual Vec3i getIndices();
};

class AABBLeaf : public AABBNode
{
private:
	int a, b, c;

public:
	AABBLeaf(int a_, int b_, int c_);

	virtual void computeBounds(Mesh *m);

	virtual Vec3i getIndices();
};

struct AABBTreeInfo {

	int maxTreeDepth;		
	int leafVertexCount;		  
};

#endif // AABBTREE_H
