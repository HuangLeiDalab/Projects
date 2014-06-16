#include "mesh.h"

#include <QFile>
#include <QTextStream>

#include "objloader.h"

bool Mesh::initFromFile(QString filename)
{
	return ObjLoader::loadObj(filename, this);
}

void Mesh::draw( bool withColor )
{
	glDisable(GL_LIGHTING);
	for (size_t gn = 0; gn < groups.size(); gn++)
	{
		glBegin(GL_TRIANGLES);
		for (size_t tn = 0; tn < groups[gn].tIndices.size(); tn++)
		{
			TriangleFace face = faces[groups[gn].tIndices[tn]];

			for (int i = 0; i < 3; i++)
			{
				Vec3b color;
				if (withColor)
				{
					color = ps.pts[face.v[i]].c;
				}
				else
				{
					color = Vec3b(248, 197, 100);
				}
				glColor3ub(color[0], color[1], color[2]);
				if (face.hasN)
				{
					Vec3d normal = normals[face.n[i]];
					glNormal3d(normal[0], normal[1], normal[2]);
				}
				Vec3d vertex = ps.pts[face.v[i]].cp;
				glVertex3d(vertex[0], vertex[1], vertex[2]);
			}
		}
		glEnd();
	}
	glEnable(GL_LIGHTING);
}

cv::Vec3d Mesh::getNormal( int index )
{
	assert(index >= 0 && index < normals.count());
	return normals[index];
}

cv::Vec2d Mesh::getTexture( int index )
{
	assert(index >= 0 && index < textures.count());
	return textures[index];
}

TriangleFace& Mesh::getFace( int index )
{
	assert(index >= 0 && index < faces.count());
	return faces[index];
}

Group* Mesh::addGroup( Group p )
{
	groups.append(p);
	return &groups[groups.count() - 1];
}

Material Mesh::getMaterial( int index )
{
	assert(index >= 0 && index < materials.count());
	return materials[index];
}

Material* Mesh::addMaterial( Material material )
{
	materials.append(material);
	return &materials[materials.count() - 1];
}

void Mesh::buildAABBTree()
{
	AABBTreeInfo aabbTreeInfo;
	aabbTreeInfo.leafVertexCount = 3;
	aabbTreeInfo.maxTreeDepth = 18;

	bboxTree = buildAABBTree(0, faces, aabbTreeInfo);
}

void Mesh::drawAABB( int maxDepth, Vec3d color )
{
	drawAABB(bboxTree, maxDepth, color);
}

void Mesh::updateAABBTree()
{
	bboxTree->computeBounds(this);
}

void Mesh::initializeNeighborTriangles()
{
	for (int i = 0; i < faces.count(); i++)
	{
		if (faces[i].neighborCnt == 3)
		{
			continue;
		}
		int v[3];
		v[0] = faces[i].v[0];
		v[1] = faces[i].v[1];
		v[2] = faces[i].v[2];
		Edge e[3];
		e[0] = Edge(v[0], v[1]);
		e[1] = Edge(v[1], v[2]);
		e[2] = Edge(v[0], v[2]);

		for (int j = i + 1; j < faces.count() && faces[i].neighborCnt < 3; j++)
		{
			if (faces[j].neighborCnt == 3)
			{
				continue;
			}
			v[0] = faces[j].v[0];
			v[1] = faces[j].v[1];
			v[2] = faces[j].v[2];
			Edge e_[3];
			e_[0] = Edge(v[0], v[1]);
			e_[1] = Edge(v[1], v[2]);
			e_[2] = Edge(v[0], v[2]);
			int m, n;
			bool found = false;
			for (m = 0; m < 3; m++)
			{
				for (n = 0; n < 3; n++)
				{
					if (e[m] == e_[n])
					{
						faces[i].neighbor[m] = &faces[j];
						faces[j].neighbor[n] = &faces[i];
						faces[i].neighborCnt++;
						faces[j].neighborCnt++;
						found = true;
						break;
					}
				}
				if (found)
				{
					break;
				}
			}
		}
	}
}

void Mesh::setVelocity( Vec3d v )
{
	for (size_t i = 0; i < ps.pts.size(); i++)
	{
		ps.pts[i].v = v;
	}
}

void Mesh::calculateBoundingBox( cv::Vec3d &inf, 
	cv::Vec3d &sup, QVector<TriangleFace> faces_ )
{
	if (faces_.count() == 0)
	{
		return;
	}

	TriangleFace face = faces_[0];
	inf = ps.pts[face.v[0]].cp;
	sup = ps.pts[face.v[0]].cp;

	for (int i = 0; i < faces_.count(); i++)
	{
		TriangleFace face = faces_[i];

		MIN_VEC3(inf, ps.pts[face.v[0]].cp, inf);
		MIN_VEC3(inf, ps.pts[face.v[1]].cp, inf);
		MIN_VEC3(inf, ps.pts[face.v[2]].cp, inf);
		MAX_VEC3(sup, ps.pts[face.v[0]].cp, sup);
		MAX_VEC3(sup, ps.pts[face.v[1]].cp, sup);
		MAX_VEC3(sup, ps.pts[face.v[2]].cp, sup);
	}
}

int Mesh::findBestAABBTreeAxis( 
	QVector<TriangleFace> faces_, cv::Vec3d &bbxCenter )
{
	// axis loop (0 = x, 1 = y, 2 = z)
	int axis = 0;
	// number of triangles on the left/right side
	int left = 0;
	int right = 0;
	// the difference between the number of left and right triangles for each axis
	int axisValues[3];
	cv::Vec3d triangleCenter;

	// for each axis (x, y, z)
	for (axis = 0; axis < 3; axis++) 
	{
		left = 0;
		right = 0;

		// for each triangle
		for (size_t i = 0; i < faces_.size(); i++) 
		{
			TriangleFace face = faces_[i];
			triangleCenter = (ps.pts[face.v[0]].cp 
				+ ps.pts[face.v[1]].cp + ps.pts[face.v[2]].cp) / 3.0;

			// triangle falls on left side of model for axis
			if (triangleCenter[axis] <= bbxCenter[axis])
			{
				left++;
			}
			// triangle falls on right side of model for axis
			else
			{
				right++;
			}
		}

		// save the difference between left and right triangle counts for each axis
		axisValues[axis] = abs(left - right);
	}

	// set first axis (x) as current lowest difference between left and right
	int index = 0;
	int result = axisValues[0];

	// find axis which has the lowest difference in the number of left and right triangles
	for (int i = 1; i < 3; i++) 
	{
		if (axisValues[i] < result) 
		{
			result = axisValues[i];
			index = i;
		}
	}  

	return index;
}

AABBNode* Mesh::buildAABBTree( int depth, 
	QVector<TriangleFace> faces_, AABBTreeInfo &treeInfo )
{
	// number of vertices
	int vertexCount = faces_.size() * 3;

	AABBNode *aabbNode;

	if (vertexCount > treeInfo.leafVertexCount && 
		depth <= treeInfo.maxTreeDepth)
	{
		// create AABBNode
		aabbNode = new AABBNode;
		aabbNode->inf = V3dZero;
		aabbNode->sup = V3dZero;
		aabbNode->depth = depth;
		aabbNode->vertexCount = vertexCount;

		// get the AABB for the current AABB tree node - assign to aabb node
		calculateBoundingBox(aabbNode->inf, aabbNode->sup, faces_);

		// get center
		Vec3d bbxCenter = (aabbNode->inf + aabbNode->sup) / 2.0;

		// find the best axis to split the mesh
		int axis = findBestAABBTreeAxis(faces_, bbxCenter); 

		// left and ride side vertex vectors
		QVector<TriangleFace> leftSide;
		QVector<TriangleFace> rightSide;  

		// split mesh
		for (size_t i = 0; i < faces_.size(); i++) 
		{
			TriangleFace face = faces_[i];
			Vec3d triangleCenter = (ps.pts[face.v[0]].cp 
				+ ps.pts[face.v[1]].cp + ps.pts[face.v[2]].cp) / 3.0;

			// triangle to left
			if (triangleCenter[axis] <= bbxCenter[axis]) 
			{
				leftSide.append(face);
			} 
			// triangle to right
			else 
			{
				rightSide.append(face);
			}  
		}

		// build left child nodes
		if (leftSide.size() > 0) 
		{
			aabbNode->left = buildAABBTree(depth + 1, leftSide, treeInfo);
		} 
		else 
		{
			aabbNode->left = NULL;
		}

		// build right child nodes
		if (rightSide.size() > 0) 
		{
			aabbNode->right = buildAABBTree(depth + 1, rightSide, treeInfo);
		} 
		else 
		{
			aabbNode->right = NULL;
		}
	}
	else // leaf node
	{
		TriangleFace face = faces_[0];
		aabbNode = new AABBLeaf(face.v[0], face.v[1], face.v[2]);
		aabbNode->depth = depth;
		aabbNode->vertexCount = vertexCount;
		aabbNode->computeBounds(this);
	}

	return aabbNode;
}

void Mesh::drawAABB( AABBNode *node, int maxDepth, Vec3d color )
{
	if (node != NULL)
	{
		if (node->depth < maxDepth)
		{
			drawAABB(node->left, maxDepth, color);
			drawAABB(node->right, maxDepth, color);
		}
		else
		{
			glColor3dv(&color[0]);
			node->render();
		}
	}
}

const int dcoord[6][2] = 
	{{0, 1}, {1, 0}, {1, 1}, {1, -1}, {0, 2}, {2, 0}};
const double ks[6] = 
	{ksStruct, ksStruct, ksShear, ksShear, ksBend, ksBend};
const double kd[6] = 
	{kdStruct, kdStruct, kdShear, kdShear, kdBend, kdBend};
const Spring::SpringType sTypes[6] = {
	Spring::SPRING_STRUCTURAL,
	Spring::SPRING_STRUCTURAL,
	Spring::SPRING_SHEAR, 
	Spring::SPRING_SHEAR, 
	Spring::SPRING_BEND,
	Spring::SPRING_BEND};

void Cloth::initFromImage( QString filename, bool inverseTriangle )
{
	Mat image = imread(filename.toStdString(), -1);
	Mat indexMap(image.rows, image.cols, CV_32SC1, Scalar::all(-1));

	int v = image.rows;
	int u = image.cols;
	int cnt = 0;
	for (int r = 0; r < v; r++)
	{
		for (int c = 0; c < u; c++)
		{
			Vec4b pixel = image.at<Vec4b>(r, c);
			if (pixel[3] != 0)
			{
				Particle p;
				p.cp = Vec3d((double(c) / u - 0.5) * clothWidth, 
					topHeight - double(r) / v * clothHeight, zbuffer);
				p.op = p.cp;
				p.m = mass;
				p.c = Vec3b(pixel[2], pixel[1], pixel[0]);
				if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)
				{
					p.c = color;
					markedVertex.append(cnt);
				}
				if (r < 9)
				{
					spinedVertex.append(cnt);
				}
				addParticle(p);
				indexMap.at<int>(r, c) = cnt++;
			}
		}
	}

	// setup sprints and triangles
	for (int r = 0; r < v; r++)
	{
		for (int c = 0; c < u; c++)
		{
			int off0 = indexMap.at<int>(r, c);
			if (off0 < 0)
			{
				continue;
			}

			int neighbors[6];
			for (int i = 0; i < 6; i++)
			{
				int nr = r + dcoord[i][0];
				int nc = c + dcoord[i][1];
				neighbors[i] = -1;
				if (nr >= 0 && nr < v && nc >= 0 && nc < u)
				{
					int off1 = indexMap.at<int>(nr, nc);
					if (off1 >= 0)
					{
						neighbors[i] = off1;
						ps.addSpring(off0, off1, 
							ks[i], kd[i], sTypes[i]);
					}
				}
			}

			constructFaces(off0, neighbors, inverseTriangle);
		}
	}
}

void Cloth::draw( bool )
{
	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	for (size_t tn = 0; tn < faces.size(); tn++)
	{
		TriangleFace face = faces[tn];

		for (int i = 0; i < 3; i++)
		{
			Vec3b color = ps.pts[face.v[i]].c;
			glColor3ub(color[0], color[1], color[2]);
			Vec3d vertex = ps.pts[face.v[i]].cp;
			glVertex3d(vertex[0], vertex[1], vertex[2]);
		}
	}
	glEnd();
	glEnable(GL_LIGHTING);
}

void Cloth::drawSpring()
{
	glDisable(GL_LIGHTING);
	for (size_t i = 0; i < ps.springs.size(); i++)
	{
		Spring s = ps.springs[i];
		Vec3b c;
		if (s.type == Spring::SPRING_STRUCTURAL)
		{
			c = Vec3b(255, 0, 0);
		}
		else if (s.type == Spring::SPRING_SHEAR)
		{
			c = Vec3b(0, 255, 0);
			continue;
		}
		else
		{
			c = Vec3b(0, 0, 255);
			continue;
		}
		glColor3ubv(&c[0]);
		glBegin(GL_LINES);
		glVertex3dv(&ps.pts[s.p1].cp[0]);
		glVertex3dv(&ps.pts[s.p2].cp[0]);
		glEnd();
	}
	glEnable(GL_LIGHTING);
}

void Cloth::save( QString filename )
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}
	QTextStream out(&file);
	out << QString("sc %1 %2\n").arg(ps.maxSpringLen).arg(ps.minSpringLen);
	for (size_t i = 0; i < ps.pts.size(); i++)
	{
		Vec3d p = ps.pts[i].cp;
		Vec3b c = ps.pts[i].c;
		out << QString("v %1 %2 %3\n").arg(p[0]).arg(p[1]).arg(p[2]);
		out << QString("vc %1 %2 %3\n").arg(c[0]).arg(c[1]).arg(c[2]);
	}
	for (int i = 0; i < faces.count(); i++)
	{
		TriangleFace f = faces[i];
		out << QString("f %1 %2 %3\n").arg(f.v[0])
			.arg(f.v[1]).arg(f.v[2]);
	}
	for (size_t i = 0; i < ps.springs.size(); i++)
	{
		Spring s = ps.springs[i];
		out << QString("s %1 %2 %3 %4 %5 %6\n").arg(s.p1).arg(s.p2)
			.arg(s.l0).arg(s.ks).arg(s.kd).arg(int(s.type));
	}
	out << "spin ";
	for (int i = 0; i < spinedVertex.count(); i++)
	{
		out << QString("%1 ").arg(spinedVertex[i]);
	}
	out << "\n";
	file.close();
}

void Cloth::constructFaces( int off0, int neighbors[5], bool inverseTriangle )
{
	TriangleFace f;
	if (neighbors[0] >= 0 && 
		neighbors[1] >= 0 && 
		neighbors[2] >= 0)
	{
		if (inverseTriangle)
		{
			f.v[0] = off0; f.v[1] = neighbors[2]; f.v[2] = neighbors[1];
			addFace(f);
			f.v[0] = off0; f.v[1] = neighbors[0]; f.v[2] = neighbors[2];
			addFace(f);
		}
		else
		{
			f.v[0] = off0; f.v[1] = neighbors[1]; f.v[2] = neighbors[2];
			addFace(f);
			f.v[0] = off0; f.v[1] = neighbors[2]; f.v[2] = neighbors[0];
			addFace(f);
		}
	}
	else if (neighbors[0] >= 0 && neighbors[1] >= 0)
	{
		if (inverseTriangle)
		{
			f.v[0] = off0; f.v[1] = neighbors[0]; f.v[2] = neighbors[1];
		}
		else
		{
			f.v[0] = off0; f.v[1] = neighbors[1]; f.v[2] = neighbors[0];
		}
		addFace(f);
	}
	else if (neighbors[0] >= 0 && neighbors[2] >= 0)
	{
		if (inverseTriangle)
		{
			f.v[0] = off0; f.v[1] = neighbors[0]; f.v[2] = neighbors[2];
		}
		else
		{
			f.v[0] = off0; f.v[1] = neighbors[2]; f.v[2] = neighbors[0];
		}
		addFace(f);
	}
	else if (neighbors[1] >= 0 && neighbors[2] >= 0)
	{
		if (inverseTriangle)
		{
			f.v[0] = off0; f.v[1] = neighbors[2]; f.v[2] = neighbors[1];
		}
		else
		{
			f.v[0] = off0; f.v[1] = neighbors[1]; f.v[2] = neighbors[2];
		}
		addFace(f);
	}
}

void SewMesh::initialize( Cloth *front, Cloth *back )
{
	cloth0 = front;
	cloth1 = back;
	if (front->markedVertex.count() > back->markedVertex.count())
	{
		cloth0 = back;
		cloth1 = front;
	}

	QMap<int, int> temp;
	for (int i = 0; i < cloth0->markedVertex.count(); i++)
	{
		int idx = cloth0->markedVertex[i];
		mappingIndex[idx] = getClosetIndex(idx, cloth0, cloth1);
	}
	for (int i = 0; i < cloth1->markedVertex.count(); i++)
	{
		int idx = cloth1->markedVertex[i];
		temp[idx] = getClosetIndex(idx, cloth1, cloth0);
	}
	QList<int> keys = mappingIndex.keys();
	for (int i = 0; i < keys.count(); i++)
	{
		int key = keys[i];
		int value = mappingIndex.value(key);
		if (temp[value] != key)
		{
			mappingIndex.remove(key);
		}
	}
}

void SewMesh::setUserForce()
{
/*#define SEW_EPS 0.001*/
#define SEW_EPS 0.02
	QList<int> keys = mappingIndex.keys();
	double dis = 0;
	Vec3d dir(0.0, 0.0, 0.0);
	for (int i = 0; i < keys.count(); i++)
	{
		int key = keys[i];
		int value = mappingIndex.value(key);
		Particle p0 = cloth0->ps.pts[key];
		Particle p1 = cloth1->ps.pts[value];
		Vec3d dp = p0.cp - p1.cp;
		double d = sqrt(dp.ddot(dp));

		dir += dp / d;
		if (d < EPS)
		{
			cloth0->ps.pts[key].v = V3dZero;
			cloth1->ps.pts[value].v = V3dZero;
		}
		else
		{
			//dp = 0.25 * dp / d;
			dp = 0.5 * dp / d;
			cloth0->ps.pts[key].v = -dp;
			cloth1->ps.pts[value].v = dp;
			if (cloth0->ps.pts[key].cp[0] < 0.0)
			{
				cloth0->ps.pts[key].v += Vec3d(-0.25, 0.0, 0.0);
			}
			else
			{
				cloth0->ps.pts[key].v += Vec3d(0.25, 0.0, 0.0);
			}
			if (cloth1->ps.pts[value].cp[0] < 0.0)
			{
				cloth1->ps.pts[value].v += Vec3d(-0.25, 0.0, 0.0);
			}
			else
			{
				cloth1->ps.pts[value].v += Vec3d(0.25, 0.0, 0.0);
			}
		}
		dis += d;
	}
	dir /= keys.count();
	dis /= keys.count();
	if (dis < SEW_EPS)
	{
		emit sewFinished();
	}
	for (size_t i = 0; i < cloth0->ps.pts.size(); i++)
	{
		//cloth0->ps.pts[i].v = -dir / 4;
		cloth0->ps.pts[i].fu = -/*cloth0->ps.pts[i].m **/ mg - airDamping * cloth0->ps.pts[i].v;
	}
	for (size_t i = 0; i < cloth1->ps.pts.size(); i++)
	{
		//cloth1->ps.pts[i].v = dir / 4;
		cloth1->ps.pts[i].fu = -/*cloth1->ps.pts[i].m **/ mg - airDamping * cloth1->ps.pts[i].v;
	}
}

void SewMesh::getMergedCloth( Cloth *cloth )
{
	for (size_t i = 0; i < cloth1->ps.pts.size(); i++)
	{
		Particle p = cloth1->ps.pts[i];
		p.fu = V3dZero;
		p.v = V3dZero;
		p.op = p.cp;
		p.isPinned = false;
		cloth->addParticle(p);
	}
	for (size_t i = 0; i < cloth1->ps.springs.size(); i++)
	{
		cloth->ps.springs.push_back(cloth1->ps.springs[i]);
	}
	for (int i = 0; i < cloth1->getFacesCount(); i++)
	{
		cloth->addFace(cloth1->getFace(i));
	}

	int particleBase = cloth->ps.pts.size();
	int cnt = 0;
	QMap<int, int> mapping;
	for (size_t i = 0; i < cloth0->ps.pts.size(); i++)
	{
		if (mappingIndex.contains(i))
		{
			int idx = mappingIndex[i];
			Vec3d p0 = cloth0->ps.pts[i].cp;
			Vec3d p1 = cloth1->ps.pts[idx].cp;

			Vec3d dp = p1 - p0;
			double dis = sqrt(dp.ddot(dp));
// 			if (dis < 0.01)
// 			{
				cloth->ps.pts[idx].cp = (p0 + p1) / 2;
				cloth->ps.pts[idx].op = cloth->ps.pts[idx].cp;

				mapping[i] = mappingIndex[i];

				continue;
/*			}*/
		}
		mapping[i] = particleBase + cnt++;
		Particle p = cloth0->ps.pts[i];
		p.fu = V3dZero;
		p.v = V3dZero;
		p.op = p.cp;
		p.isPinned = false;
		cloth->addParticle(p);
	}
	for (size_t i = 0; i < cloth0->ps.springs.size(); i++)
	{
		Spring s = cloth0->ps.springs[i];
		s.p1 = mapping[s.p1];
		s.p2 = mapping[s.p2];
		cloth->ps.springs.push_back(s);
	}
	for (int i = 0; i < cloth0->getFacesCount(); i++)
	{
		TriangleFace f = cloth0->getFace(i);
		f.v[0] = mapping[f.v[0]];
		f.v[1] = mapping[f.v[1]];
		f.v[2] = mapping[f.v[2]];
		cloth->addFace(f);
	}
}

void SewMesh::drawLines()
{
	QList<int> keys = mappingIndex.keys();
	for (int i = 0; i < keys.count(); i++)
	{
		int key = keys[i];
		int value = mappingIndex.value(key);

		Vec3d p0 = cloth0->ps.pts[key].cp;
		Vec3d p1 = cloth1->ps.pts[value].cp;

		glColor3ub(0, 255, 0);
		glBegin(GL_LINES);
		glVertex3dv(&p0[0]);
		glVertex3dv(&p1[0]);
		glEnd();
	}
}

int SewMesh::getClosetIndex( int idx, Cloth *c0, Cloth *c1 )
{
	Vec3d p = c0->ps.pts[idx].cp;
	double distance = 10000.0;
	int mappingIdx = -1;
	for (int i = 0; i < c1->markedVertex.count(); i++)
	{
		Vec3d mp = c1->ps.pts[c1->markedVertex[i]].cp;

		Vec3d d = p - mp;
		double dis = sqrt(d.ddot(d));
		if (dis < distance)
		{
			distance = dis;
			mappingIdx = c1->markedVertex[i];
		}
	}
	return mappingIdx;
}

void SewSkirtMesh::setUserForce()
{
#define SEW_EPS 0.04
	QList<int> keys = mappingIndex.keys();
	double dis = 0;
	Vec3d dir(0.0, 0.0, 0.0);
	for (int i = 0; i < keys.count(); i++)
	{
		int key = keys[i];
		int value = mappingIndex.value(key);
		Particle p0 = cloth0->ps.pts[key];
		Particle p1 = cloth1->ps.pts[value];
		Vec3d dp = p0.cp - p1.cp;
		double d = sqrt(dp.ddot(dp));

		dir += dp / d;
		if (d < EPS)
		{
			cloth0->ps.pts[key].v = V3dZero;
			cloth1->ps.pts[value].v = V3dZero;
		}
		else
		{
			dp = 0.5 * dp / d;
			cloth0->ps.pts[key].v = -dp;
			cloth1->ps.pts[value].v = dp;
			if (cloth0->ps.pts[key].cp[0] < 0.0)
			{
				cloth0->ps.pts[key].v += Vec3d(-0.25, 0.0, 0.0);
			}
			else
			{
				cloth0->ps.pts[key].v += Vec3d(0.25, 0.0, 0.0);
			}
			if (cloth1->ps.pts[value].cp[0] < 0.0)
			{
				cloth1->ps.pts[value].v += Vec3d(-0.25, 0.0, 0.0);
			}
			else
			{
				cloth1->ps.pts[value].v += Vec3d(0.25, 0.0, 0.0);
			}
		}
		dis += d;
	}
	dir /= keys.count();
	dis /= keys.count();
	if (dis < SEW_EPS)
	{
		emit sewFinished();
	}
	for (size_t i = 0; i < cloth0->ps.pts.size(); i++)
	{
		//cloth0->ps.pts[i].v = -dir / 4;
		cloth0->ps.pts[i].fu = -/*cloth0->ps.pts[i].m **/ mg - airDamping * cloth0->ps.pts[i].v;
	}
	for (size_t i = 0; i < cloth1->ps.pts.size(); i++)
	{
		//cloth1->ps.pts[i].v = dir / 4;
		cloth1->ps.pts[i].fu = -/*cloth1->ps.pts[i].m **/ mg - airDamping * cloth1->ps.pts[i].v;
	}
}

void SewSkirtMesh::getMergedCloth( Cloth *cloth )
{
	for (size_t i = 0; i < cloth1->ps.pts.size(); i++)
	{
		Particle p = cloth1->ps.pts[i];
		p.fu = V3dZero;
		p.v = V3dZero;
		p.op = p.cp;
		p.isPinned = false;
		cloth->addParticle(p);
	}
	for (size_t i = 0; i < cloth1->ps.springs.size(); i++)
	{
		cloth->ps.springs.push_back(cloth1->ps.springs[i]);
	}
	for (int i = 0; i < cloth1->getFacesCount(); i++)
	{
		cloth->addFace(cloth1->getFace(i));
	}

	int particleBase = cloth->ps.pts.size();
	int cnt = 0;
	QMap<int, int> mapping;
	for (size_t i = 0; i < cloth0->ps.pts.size(); i++)
	{
		if (mappingIndex.contains(i))
		{
			int idx = mappingIndex[i];
			Vec3d p0 = cloth0->ps.pts[i].cp;
			Vec3d p1 = cloth1->ps.pts[idx].cp;

			Vec3d dp = p1 - p0;
			double dis = sqrt(dp.ddot(dp));

			cloth->ps.pts[idx].cp = (p0 + p1) / 2;
			cloth->ps.pts[idx].op = cloth->ps.pts[idx].cp;

			mapping[i] = mappingIndex[i];

			continue;
		}
		mapping[i] = particleBase + cnt++;
		Particle p = cloth0->ps.pts[i];
		p.fu = V3dZero;
		p.v = V3dZero;
		p.op = p.cp;
		p.isPinned = false;
		cloth->addParticle(p);
	}
	for (size_t i = 0; i < cloth0->ps.springs.size(); i++)
	{
		Spring s = cloth0->ps.springs[i];
		s.p1 = mapping[s.p1];
		s.p2 = mapping[s.p2];
		cloth->ps.springs.push_back(s);
	}
	for (int i = 0; i < cloth0->getFacesCount(); i++)
	{
		TriangleFace f = cloth0->getFace(i);
		f.v[0] = mapping[f.v[0]];
		f.v[1] = mapping[f.v[1]];
		f.v[2] = mapping[f.v[2]];
		cloth->addFace(f);
	}

	for (int i = 0; i < cloth1->spinedVertex.count(); i++)
	{
		int idx = cloth1->spinedVertex[i];
		cloth->spinedVertex.append(idx);
		cloth->ps.pts[idx].isPinned = true;
	}
	for (int i = 0; i < cloth0->spinedVertex.count(); i++)
	{
		int idx = mapping[cloth0->spinedVertex[i]];
		if (!cloth->spinedVertex.contains(idx))
		{
			cloth->spinedVertex.append(idx);
			cloth->ps.pts[idx].isPinned = true;
		}
	}
}

void SewTshirtMesh::setUserForce()
{
//#define SEW_EPS1 0.02
#define SEW_EPS2 0.01
	QList<int> keys = mappingIndex.keys();
	double dis = 0;
	Vec3d dir(0.0, 0.0, 0.0);
	for (int i = 0; i < keys.count(); i++)
	{
		int key = keys[i];
		int value = mappingIndex.value(key);
		Particle p0 = cloth0->ps.pts[key];
		Particle p1 = cloth1->ps.pts[value];
		Vec3d dp = p0.cp - p1.cp;
		double d = sqrt(dp.ddot(dp));

		dir += dp / d;
		if (d < EPS)
		{
			cloth0->ps.pts[key].v = V3dZero;
			cloth1->ps.pts[value].v = V3dZero;
		}
		else
		{
			dp = 0.5 * dp / d;
			cloth0->ps.pts[key].v = -dp;
			cloth1->ps.pts[value].v = dp;
			if (cloth0->ps.pts[key].cp[0] < 0.0)
			{
				cloth0->ps.pts[key].v += Vec3d(-0.25, 0.0, 0.0);
			}
			else
			{
				cloth0->ps.pts[key].v += Vec3d(0.25, 0.0, 0.0);
			}
			if (cloth1->ps.pts[value].cp[0] < 0.0)
			{
				cloth1->ps.pts[value].v += Vec3d(-0.25, 0.0, 0.0);
			}
			else
			{
				cloth1->ps.pts[value].v += Vec3d(0.25, 0.0, 0.0);
			}
		}
		dis += d;
	}
	dir /= keys.count();
	dis /= keys.count();
	if (dis < SEW_EPS2)
	{
		emit sewFinished();
	}
	Vec3d gravity = mg;
// 	if (dis < SEW_EPS1)
// 	{
// 		gravity = V3dZero;
// 	}
	for (size_t i = 0; i < cloth0->ps.pts.size(); i++)
	{
		cloth0->ps.pts[i].fu = -gravity - airDamping * cloth0->ps.pts[i].v;
	}
	for (size_t i = 0; i < cloth1->ps.pts.size(); i++)
	{
		cloth1->ps.pts[i].fu = -gravity - airDamping * cloth1->ps.pts[i].v;
	}
}

void SewTshirtMesh::getMergedCloth( Cloth *cloth )
{
	for (size_t i = 0; i < cloth1->ps.pts.size(); i++)
	{
		Particle p = cloth1->ps.pts[i];
		p.fu = V3dZero;
		p.v = V3dZero;
		p.op = p.cp;
		p.isPinned = false;
		cloth->addParticle(p);
	}
	for (size_t i = 0; i < cloth1->ps.springs.size(); i++)
	{
		cloth->ps.springs.push_back(cloth1->ps.springs[i]);
	}
	for (int i = 0; i < cloth1->getFacesCount(); i++)
	{
		cloth->addFace(cloth1->getFace(i));
	}

	int particleBase = cloth->ps.pts.size();
	int cnt = 0;
	QMap<int, int> mapping;
	for (size_t i = 0; i < cloth0->ps.pts.size(); i++)
	{
		if (mappingIndex.contains(i))
		{
			int idx = mappingIndex[i];
			Vec3d p0 = cloth0->ps.pts[i].cp;
			Vec3d p1 = cloth1->ps.pts[idx].cp;

			Vec3d dp = p1 - p0;
			double dis = sqrt(dp.ddot(dp));

			cloth->ps.pts[idx].cp = (p0 + p1) / 2;
			cloth->ps.pts[idx].op = cloth->ps.pts[idx].cp;

			mapping[i] = mappingIndex[i];

			continue;
		}
		mapping[i] = particleBase + cnt++;
		Particle p = cloth0->ps.pts[i];
		p.fu = V3dZero;
		p.v = V3dZero;
		p.op = p.cp;
		p.isPinned = false;
		cloth->addParticle(p);
	}
	for (size_t i = 0; i < cloth0->ps.springs.size(); i++)
	{
		Spring s = cloth0->ps.springs[i];
		s.p1 = mapping[s.p1];
		s.p2 = mapping[s.p2];
		cloth->ps.springs.push_back(s);
	}
	for (int i = 0; i < cloth0->getFacesCount(); i++)
	{
		TriangleFace f = cloth0->getFace(i);
		f.v[0] = mapping[f.v[0]];
		f.v[1] = mapping[f.v[1]];
		f.v[2] = mapping[f.v[2]];
		cloth->addFace(f);
	}
}
