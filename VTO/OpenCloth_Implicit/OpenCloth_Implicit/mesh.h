#ifndef MESH_H
#define MESH_H

#include "aabbtree.h"
#include "utils.h"



struct TriangleFace   
{  
	int indices[3]; // vertex indices
	TriangleFace *neighbor[3];
	int neighborCnt;
	int index;

	TriangleFace()
	{
		neighbor[0] = NULL;
		neighbor[1] = NULL;
		neighbor[2] = NULL;
		neighborCnt = 0;
		index = -1;
	}
};

struct Edge
{
	int v[2];

	Edge()
	{
		v[0] = -1;
		v[1] = -1;
	}

	Edge(int v0, int v1)
	{
		if (v0 < v1)
		{
			v[0] = v0;
			v[1] = v1;
		}
		else
		{
			v[0] = v1;
			v[1] = v0;
		}
	}
	bool operator==(const Edge &e)
	{
		return (v[0] == e.v[0] && v[1] == e.v[1]);
	}
};

class AABBNode;
struct AABBTreeInfo;

class Mesh  
{  
	friend class hashSpatial;
public:
	AABBNode *bboxTree;

	Mesh(vec3Array& vertices, const uvec3Array &indices);
	Mesh(vec3Array& vertices, const std::vector<unsigned int> &indices);

	~Mesh(){}
	virtual void draw(bool withColor = false);

	void setVertices(vec3Array &vertices) { mVertices = vertices; }

	void initializeMesh(vec3Array& vertices, const uvec3Array &indices);

	void initializeMesh(vec3Array& vertices, const std::vector<GLushort> &indices);

	void buildAABBTree();
	
	void updateAABBTree();

	void initializeNeighborTriangles();

	void drawAABB(int maxDepth, vec3 color);

protected:

	void calculateBoundingBox(vec3 &inf, vec3 &sup, const std::vector<TriangleFace>& faces);

	int findBestAABBTreeAxis(const std::vector<TriangleFace>& faces, vec3 &bbxCenter);

	AABBNode *buildAABBTree(int depth,const std::vector<TriangleFace>& faces, AABBTreeInfo &treeInfo);

	void drawAABB(AABBNode *node, int maxDepth, vec3 color);

public:
	vec3Array& mVertices;
	std::vector<TriangleFace> mFaces;
};

/*class Cloth : public Mesh
{
public:
	Cloth(Vec3b c = Vec3b(255, 255, 255), double z = 1.0, 
		double cw = g_cw, double ch = g_ch, double th = 1.63)
		: clothWidth(cw), clothHeight(ch), topHeight(th), 
		zbuffer(z), color(c){}
	void initFromImage(QString filename, bool inverseTriangle = false);
	void setTexture(GLuint t){ texture = t; }

	void draw(bool d = false);
	void drawSpring();

	void save(QString filename);

private:
	double clothWidth;
	double clothHeight, topHeight;
	double zbuffer;
	Vec3b color;
	GLuint texture;

	QVector<int> markedVertex;
	QVector<int> spinedVertex;

	void constructFaces(int off0, int neighbors[5], bool inverseTriangle = false);

	friend class SewMesh;
	friend class SewSkirtMesh;
	friend class SewTshirtMesh;
};

class SewMesh : public QObject
{
	Q_OBJECT

public:
	Cloth *cloth0, *cloth1;
	QMap<int, int> mappingIndex;

	SewMesh(QObject *parent = 0) : QObject(parent){}

	void initialize(Cloth *front, Cloth *back);
	virtual void setUserForce();

	virtual void getMergedCloth(Cloth *cloth);

	void drawLines();

signals:
	void sewFinished();

private:
	int getClosetIndex(int idx, Cloth *c0, Cloth *c1);
};

class SewSkirtMesh : public SewMesh
{
public:
	void setUserForce();

	void getMergedCloth(Cloth *cloth);
};

class SewTshirtMesh : public SewMesh
{
	void setUserForce();

	void getMergedCloth(Cloth *cloth);
};*/


#endif