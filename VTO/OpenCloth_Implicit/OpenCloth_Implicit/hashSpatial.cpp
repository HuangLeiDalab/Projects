#include "hashSpatial.h"
#include <iostream>

hashSpatial::hashSpatial( Mesh *mesh )
	:mMesh(mesh)
{
	mTable.resize(HASH_TABLE_SIZE);
	hashIndice indiceArr;
	for (int i = 0; i < HASH_TABLE_SIZE; ++i){
		mTable[i] = indiceArr;
	}
	buildHashTable();
}

hashSpatial::~hashSpatial()
{
	clearHashTable();
	if(mMesh != NULL) 
		delete mMesh; 
	mMesh = NULL; 
}


void hashSpatial::buildHashTable()
{
	mBBoxMin = mMesh->mVertices[0];
	mBBoxMax = mMesh->mVertices[0];
	mCellSize = vec3(0, 0, 0);
	mNodes.resize(mMesh->mFaces.size());
	for (int fn = 0; fn < mMesh->mFaces.size(); ++fn){
		int *indices = mMesh->mFaces[fn].indices;

		/* New hashNodes which are loaded into a List*/
		hashNode node;
		node.indices.push_back(indices[0]);
		node.indices.push_back(indices[1]);
		node.indices.push_back(indices[2]);

		node.bboxMin = mMesh->mVertices[indices[0]];
		node.bboxMax = mMesh->mVertices[indices[0]];
		MIN_VEC3(node.bboxMin, mMesh->mVertices[indices[1]], node.bboxMin);
		MIN_VEC3(node.bboxMin, mMesh->mVertices[indices[2]], node.bboxMin);
		MAX_VEC3(node.bboxMax, mMesh->mVertices[indices[1]], node.bboxMax);
		MAX_VEC3(node.bboxMax, mMesh->mVertices[indices[2]], node.bboxMax);

		MIN_VEC3(node.bboxMin, mBBoxMin, mBBoxMin);
		MAX_VEC3(node.bboxMax, mBBoxMax, mBBoxMax);

		mNodes[fn] = node;
		/* Calculate the hash cell size */
		mCellSize += (node.bboxMax - node.bboxMin);
	}
	mCellSize = mCellSize * (1.0f / mMesh->mFaces.size());

	int coord[6];
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
					int key = hashFunction(i, j, k);
					mTable[key].push_back(n);
				}
			}
		}
	}
}


void hashSpatial::updateHashTable()
{
	clearHashTable();
	buildHashTable();
}


int hashSpatial::hashFunction( const int x, const int y, const int z )
{
	unsigned long hash = x * 73856093;
	hash = hash ^ (y * 19349663);
	hash = hash ^ (z * 83492791);
	return	hash % HASH_TABLE_SIZE;
}

void hashSpatial::clearHashTable()
{
	for (int i = 0; i < HASH_TABLE_SIZE; ++i){
		mTable[i].clear();
	}
}

int hashSpatial::intersect( const vec3& p )
{
	if (p.x < mBBoxMin.x || p.y < mBBoxMin.y || p.z < mBBoxMin.z ||
		p.x > mBBoxMax.x || p.y > mBBoxMax.y || p.z > mBBoxMax.z) {
		return NULL;
	}
	int coord[3];
	coord[0] = (p.x - mBBoxMin.x) / mCellSize.x;
	coord[1] = (p.y - mBBoxMin.y) / mCellSize.y;
	coord[2] = (p.z - mBBoxMin.z) / mCellSize.z;
	return hashFunction(coord[0], coord[1], coord[2]);
}

void hashSpatial::intersect( vec3& pre, vec3& curr, vec3& vel )
{
	int cellID = intersect(curr);
	for (int n = 0; n < mTable[cellID].size(); ++n) {
		USHORT nodeID = mTable[cellID][n];
		if (curr.x < mNodes[nodeID].bboxMin.x || curr.y < mNodes[nodeID].bboxMin.y || curr.z < mNodes[nodeID].bboxMin.z ||
			curr.x > mNodes[nodeID].bboxMax.x || curr.y > mNodes[nodeID].bboxMax.y || curr.z > mNodes[nodeID].bboxMax.z) {
				continue;
		}else{
			/*if (collider::collideTriangle(
				pre, curr, vel,
				mMesh->mVertices[mNodes[nodeID].indices[0]], 
				mMesh->mVertices[mNodes[nodeID].indices[1]],
				mMesh->mVertices[mNodes[nodeID].indices[2]]))
				return;*/
			if (collider::collideTriangle(curr, vel, 
				mMesh->mVertices[mNodes[nodeID].indices[0]],
				mMesh->mVertices[mNodes[nodeID].indices[1]],
				mMesh->mVertices[mNodes[nodeID].indices[2]],
				0.001f))
				return;
		}
	}
}


void hashSpatial::drawBBox()
{
	for (int i = 0; i < HASH_TABLE_SIZE; ++i){
		 for (int j = 0; j < mTable[i].size(); j++){
			 drawBBox( mNodes[mTable[i][j]].bboxMin, mNodes[mTable[i][j]].bboxMax ) ;
		 }
	}
}

void hashSpatial::drawBBox( const vec3& min, const vec3 max )
{
	glColor3f(0.0, 0.0, 1.0);
	vec3 p0(min),
		p1(max[0], min[1], min[2]),
		p2(max[0], max[1], min[2]),
		p3(min[0], max[1], min[2]),
		p4(min[0], min[1], max[2]),
		p5(max[0], min[1], max[2]),
		p6(max),
		p7(min[0], max[1], max[2]);
	glBegin(GL_LINES);

	glVertex3fv(&p0[0]);
	glVertex3fv(&p1[0]);

	glVertex3fv(&p1[0]);
	glVertex3fv(&p2[0]);

	glVertex3fv(&p2[0]);
	glVertex3fv(&p3[0]);

	glVertex3fv(&p3[0]);
	glVertex3fv(&p0[0]);

	glVertex3fv(&p4[0]);
	glVertex3fv(&p5[0]);

	glVertex3fv(&p5[0]);
	glVertex3fv(&p6[0]);

	glVertex3fv(&p6[0]);
	glVertex3fv(&p7[0]);

	glVertex3fv(&p7[0]);
	glVertex3fv(&p4[0]);

	glVertex3fv(&p0[0]);
	glVertex3fv(&p4[0]);

	glVertex3fv(&p1[0]);
	glVertex3fv(&p5[0]);

	glVertex3fv(&p2[0]);
	glVertex3fv(&p6[0]);

	glVertex3fv(&p3[0]);
	glVertex3fv(&p7[0]);

	glEnd();

	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_POINTS);
	glVertex3fv(&p0[0]);
	glVertex3fv(&p6[0]);
	glEnd();
}

bool collider::collideTriangle( vec3& prePos, vec3& currPos, vec3& velocity, const vec3 t0, const vec3 t1, const vec3 t2 )
{
	vec3 t0t1 = t1 - t0;
	vec3 t0t2 = t2 - t0;
	vec3 currpre = prePos - currPos;
	vec3 n = glm::cross(t0t1, t0t2);
	float d = glm::dot(currpre, n);
	if (d <= 0.0f){
		return false;
	}
	vec3 t0pre = prePos - t0;
	float t = glm::dot(t0pre, n);
	if (t < 0.0f){
		return false;
	}
	if (t > d){
		return false;
	}
	vec3 e = glm::cross(currpre, t0pre);
	float v = glm::dot(t0t2, e);
	if (v < 0.0f || v > d){
		return false;
	}
	float w = -glm::dot(t0t1, e);
	if (w < 0.0f || v + w > d){
		return false;
	}
	t -= 1e-6;
	currPos = prePos - currpre * (t / d);
	
	n = glm::normalize(n);
	float tmpV = glm::dot(velocity, n);
	vec3 vn = tmpV * n;
	vec3 vt = velocity - vn;

	// new velocity = (1 - u)vt - e vn
	velocity = (0.4f) * vt - 0.1f * vn;;
	//velocity = vec3(0.0, 0.0, 0.0);
	return true;
}

bool collider::collideTriangle( vec3& pos, vec3& vel, const vec3 t0, const vec3 t1, const vec3 t2, const float h)
{
	vec3 t0t1 = t1 - t0;
	vec3 t0t2 = t2 - t0;
	vec3 t0P = pos - t0;
	vec3 n = glm::normalize(glm::cross(t0t1, t0t2));
	// The distance from P to the Triangle
	float d = glm::dot(t0P, n);
	// Check "pos - t * v" intersect with the Triangle
	float t = d / glm::dot(vel, n);
	vec3 newPos = pos - t * vel;
	vec3 p = newPos - t0;
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
		pos = pos + n * (h - d);
	}
	// compute normal and tangential components of velocity
	//vel = 0.5f * vel;
	float tmpV = glm::dot(vel, n);
	vec3 vn = tmpV * n;
	vec3 vt = vel - vn;

	// new velocity = (1 - u)vt - e vn
	vel = 0.004f * vt - 0.1f * vn;
	//vel = (0.05f) * vt - 0.8f * vn;
	//vel = vec3(0.0f, 0.0f, 0.f);
	return true;
}
