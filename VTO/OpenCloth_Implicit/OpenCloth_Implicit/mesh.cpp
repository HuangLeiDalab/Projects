#include "mesh.h"



Mesh::Mesh(vec3Array& vertices, const uvec3Array &indices)
	:mVertices(vertices)
{
	mFaces.resize(indices.size());
	for (int i = 0; i < indices.size(); ++i){
		TriangleFace newFace;
		newFace.indices[0] = indices[i].x;
		newFace.indices[1] = indices[i].y;
		newFace.indices[2] = indices[i].z;
		mFaces[i] = newFace;
	}
	initializeNeighborTriangles();
}

Mesh::Mesh(vec3Array& vertices, const std::vector<unsigned int> &indices)
	: mVertices(vertices)
{
	mFaces.resize(indices.size() / 3);
	for (int i = 0; i < mFaces.size(); ++i){
		TriangleFace newFace;
		newFace.indices[0] = indices[i * 3];
		newFace.indices[1] = indices[i * 3 + 1];
		newFace.indices[2] = indices[i * 3 + 2];
		mFaces[i] = newFace;
	}
	//initializeNeighborTriangles();
}

void Mesh::initializeMesh(vec3Array& vertices, const uvec3Array &indices)
{
	mVertices = vertices;
	mFaces.clear();
	mFaces.resize(indices.size());
	for (int i = 0; i < indices.size(); ++i){
		TriangleFace newFace;
		newFace.indices[0] = indices[i].x;
		newFace.indices[1] = indices[i].y;
		newFace.indices[2] = indices[i].z;
		mFaces[i] = newFace;
	}
	initializeNeighborTriangles();
}

void Mesh::initializeMesh(vec3Array& vertices, const std::vector<GLushort> &indices)
{
	mVertices = vertices;
	mFaces.clear();
	mFaces.resize(indices.size() / 3);
	for (int i = 0; i < indices.size(); i += 3){
		TriangleFace newFace;
		newFace.indices[0] = indices[i];
		newFace.indices[1] = indices[i + 1];
		newFace.indices[2] = indices[i + 2];
		mFaces[i] = newFace;
	}
	initializeNeighborTriangles();
}

void Mesh::draw( bool withColor )
{
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	for (size_t fn = 0; fn < mFaces.size(); ++fn){

#ifdef HIGH_PRICE
		glVertex3d(mVertices[mFaces[fn].indices[0]].x, mVertices[mFaces[fn].indices[0]].y, mVertices[mFaces[fn].indices[0]].z);
		glVertex3d(mVertices[mFaces[fn].indices[1]].x, mVertices[mFaces[fn].indices[1]].y, mVertices[mFaces[fn].indices[1]].z);
		glVertex3d(mVertices[mFaces[fn].indices[2]].x, mVertices[mFaces[fn].indices[2]].y, mVertices[mFaces[fn].indices[2]].z);
#else
		glVertex3f(mVertices[mFaces[fn].indices[0]].x, mVertices[mFaces[fn].indices[0]].y, mVertices[mFaces[fn].indices[0]].z);
		glVertex3f(mVertices[mFaces[fn].indices[1]].x, mVertices[mFaces[fn].indices[1]].y, mVertices[mFaces[fn].indices[1]].z);
		glVertex3f(mVertices[mFaces[fn].indices[2]].x, mVertices[mFaces[fn].indices[2]].y, mVertices[mFaces[fn].indices[2]].z);
#endif
		
	}
	glEnd();
}


void Mesh::buildAABBTree()
{
	AABBTreeInfo aabbTreeInfo;
	aabbTreeInfo.leafVertexCount = 3;
	aabbTreeInfo.maxTreeDepth = 18;

	bboxTree = buildAABBTree(0, mFaces, aabbTreeInfo);
}

void Mesh::drawAABB( int maxDepth, vec3 color )
{
	drawAABB(bboxTree, maxDepth, color);
}

void Mesh::updateAABBTree()
{
	bboxTree->computeBounds(this);
}

void Mesh::initializeNeighborTriangles()
{
	for (int i = 0; i < mFaces.size(); i++)
	{
		if (mFaces[i].neighborCnt == 3)
		{
			continue;
		}
		int v[3];
		v[0] = mFaces[i].indices[0];
		v[1] = mFaces[i].indices[1];
		v[2] = mFaces[i].indices[2];
		Edge e[3];
		e[0] = Edge(v[0], v[1]);
		e[1] = Edge(v[1], v[2]);
		e[2] = Edge(v[0], v[2]);

		for (int j = i + 1; j < mFaces.size() && mFaces[i].neighborCnt < 3; j++)
		{
			if (mFaces[j].neighborCnt == 3)
			{
				continue;
			}
			v[0] = mFaces[j].indices[0];
			v[1] = mFaces[j].indices[1];
			v[2] = mFaces[j].indices[2];
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
						mFaces[i].neighbor[m] = &mFaces[j];
						mFaces[j].neighbor[n] = &mFaces[i];
						mFaces[i].neighborCnt++;
						mFaces[j].neighborCnt++;
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



void Mesh::calculateBoundingBox(vec3 &inf, vec3 &sup, const std::vector<TriangleFace>& faces)
{
	if (faces.size() == 0){
		return;
	}
	inf = mVertices[faces[0].indices[0]];
	sup = mVertices[faces[0].indices[0]];
	for (int i = 0; i < faces.size(); i++)
	{
		const TriangleFace& tmpFace = faces[i];
		MIN_VEC3(inf, mVertices[tmpFace.indices[0]], inf);
		MIN_VEC3(inf, mVertices[tmpFace.indices[1]], inf);
		MIN_VEC3(inf, mVertices[tmpFace.indices[2]], inf);
		MAX_VEC3(sup, mVertices[tmpFace.indices[0]], sup);
		MAX_VEC3(sup, mVertices[tmpFace.indices[1]], sup);
		MAX_VEC3(sup, mVertices[tmpFace.indices[2]], sup);
	}
}

int Mesh::findBestAABBTreeAxis(const std::vector<TriangleFace>& faces, vec3 &bbxCenter)
{
	// axis loop (0 = x, 1 = y, 2 = z)
	int axis = 0;
	// number of triangles on the left/right side
	int left = 0;
	int right = 0;
	// the difference between the number of left and right triangles for each axis
	int axisValues[3];
	vec3 triangleCenter;

	// for each axis (x, y, z)
	for (axis = 0; axis < 3; axis++) 
	{
		left = 0;
		right = 0;

		// for each triangle
		for (size_t i = 0; i < faces.size(); i++) 
		{
			const TriangleFace& tmpFace = faces[i];
			triangleCenter = mVertices[tmpFace.indices[0]] + mVertices[tmpFace.indices[1]] + mVertices[tmpFace.indices[2]];
			triangleCenter *= 1.0 / 3.0;
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

AABBNode* Mesh::buildAABBTree(int depth, const std::vector<TriangleFace>& faces, AABBTreeInfo &treeInfo)
{
	// number of vertices
	int vertexCount = faces.size() * 3;

	AABBNode *aabbNode;

	if (vertexCount > treeInfo.leafVertexCount && depth <= treeInfo.maxTreeDepth)
	{
		// create AABBNode
		aabbNode = new AABBNode;
		aabbNode->inf = V3dZero;
		aabbNode->sup = V3dZero;
		aabbNode->depth = depth;
		aabbNode->vertexCount = vertexCount;

		// get the AABB for the current AABB tree node - assign to aabb node
		calculateBoundingBox(aabbNode->inf, aabbNode->sup, faces);

		// get center
		vec3 bbxCenter = aabbNode->inf + aabbNode->sup;
		bbxCenter *= 1.0 / 2.0;

		// find the best axis to split the mesh
		int axis = findBestAABBTreeAxis(faces, bbxCenter); 

		// left and ride side vertex vectors
		std::vector<TriangleFace> leftSide;
		std::vector<TriangleFace> rightSide;  

		// split mesh
		for (size_t i = 0; i < faces.size(); i++) 
		{
			const TriangleFace& tmpFace = faces[i];
			vec3 triangleCenter = mVertices[tmpFace.indices[0]] + mVertices[tmpFace.indices[1]] + mVertices[tmpFace.indices[2]];
			triangleCenter *= 1.0 / 3.0;
			// triangle to left
			if (triangleCenter[axis] <= bbxCenter[axis]) 
			{
				leftSide.push_back(tmpFace);
			} 
			// triangle to right
			else 
			{
				rightSide.push_back(tmpFace);
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
		const TriangleFace tmpFace = faces[0];
		aabbNode = new AABBLeaf(tmpFace.indices[0], tmpFace.indices[1], tmpFace.indices[2]);
		aabbNode->depth = depth;
		aabbNode->vertexCount = vertexCount;
		aabbNode->computeBounds(this);
	}

	return aabbNode;
}

void Mesh::drawAABB( AABBNode *node, int maxDepth, vec3 color )
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
			glColor3fv(&color[0]);
			node->render();
		}
	}
}


