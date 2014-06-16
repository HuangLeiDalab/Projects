#ifndef MESH_H
#define MESH_H

#include "particlesystem.h"
#include "aabbtree.h"

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QList>

using namespace ps;

struct Material
{
	QString name;
	GLfloat diffuse[4];
	GLfloat ambient[4];
	GLfloat specular[4];
	GLfloat shininess;

	Material()
	{
		name = "";
		shininess = 65.0f;
		diffuse[0] = 0.8f;
		diffuse[1] = 0.8f;
		diffuse[2] = 0.8f;
		diffuse[3] = 1.0f;
		ambient[0] = 0.2f;
		ambient[1] = 0.2f;
		ambient[2] = 0.2f;
		ambient[3] = 1.0f;
		specular[0] = 0.0f;
		specular[1] = 0.0f;
		specular[2] = 0.0f;
		specular[3] = 1.0f;
	}
};

struct TriangleFace   
{  
	GLuint v[3]; // vertex indices
	GLuint t[3]; // texture indices
	GLuint n[3]; // normal indices
	bool hasN;
	bool hasT;
	TriangleFace *neighbor[3];
	int neighborCnt;

	int index;

	TriangleFace()
	{
		hasN = false;
		hasT = false;
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

struct Group
{
	QString name;
	QVector<GLuint> tIndices;
	GLuint material;
};

class Mesh  
{  
public:
	ParticleSystem ps;
	AABBNode *bboxTree;

	Mesh(){}
	~Mesh(){}
	bool initFromFile(QString filename);
	virtual void draw(bool withColor = false);

	QString getPathName(){ return pathname; }
	void setPathName(QString pathname_){ pathname = pathname_; }
	QString getMtlLibName(){ return mtllibname; }
	void setMtlLibName(QString mtllibname_){ mtllibname = mtllibname_; }
	Vec3d getNormal(int index);
	void addNormal(Vec3d normal){ normals.append(normal); }
	int getNormalsCount(){ return normals.count(); }
	Vec2d getTexture(int index);
	void addTexture(Vec2d texture){ textures.append(texture); }
	int getTexturesCount(){ return textures.count(); }
	TriangleFace& getFace(int index);
	void addFace(TriangleFace face){ faces.append(face); }
	int getFacesCount(){ return faces.count(); }
	Group* addGroup(Group p);
	int getGroupsCount(){ return groups.count(); }
	Material getMaterial(int index);
	Material* addMaterial(Material material);
	int getMaterialCount(){ return materials.count(); }
	void addParticle(Particle p){ ps.pts.push_back(p); }

	void buildAABBTree();
	void drawAABB(int maxDepth, Vec3d color);
	void updateAABBTree();

	void initializeNeighborTriangles();

	void setVelocity(Vec3d v);

protected:
	QString pathname;
	QString mtllibname;
	QVector<Vec3d> normals;
	QVector<Vec2d> textures;
	QVector<TriangleFace> faces;
	QVector<Group> groups;
	QVector<Material> materials;

	void calculateBoundingBox(Vec3d &inf, Vec3d &sup, 
		QVector<TriangleFace> faces_);
	int findBestAABBTreeAxis(
		QVector<TriangleFace> faces_, Vec3d &bbxCenter);
	AABBNode *buildAABBTree(int depth, 
		QVector<TriangleFace> faces_, AABBTreeInfo &treeInfo);
	void drawAABB(AABBNode *node, int maxDepth, Vec3d color);
};

class Cloth : public Mesh
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
};


#endif