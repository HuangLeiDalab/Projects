#ifndef _CLOTH_H_
#define _CLOTH_H_

#include <vector>
#include <map>

struct clothEdge{
	int e1, e2;
	clothEdge(const int _e1, const int _e2){
		if (_e1 < _e2){
			e1 = _e1;
			e2 = _e2;
		}else{
			e1 = _e2;
			e2 = _e1;
		}
	}
	bool operator < (const clothEdge& e) const {
		if (e1 == e.e1)
			return e2 < e.e2;
		else
			return e1 < e.e1;
	}
};

struct clothPair{
	int p1, p2;
	clothPair() { p1 = -1; p2 = -1; }
	void add(const int v)
	{ 
		if (p1 != -1)
			p2 = v;
		else
			p1 = v;
	}
	bool isValid() { return p1 != -1 && p2 != -1; }
};

enum springType {
	EDGE,
	BEND
};

struct clothSpring {
	int p1, p2;
	float restLen;
	float ks, kd;
	springType type;
};

struct Cloth
{
	Cloth(int _vertexCount, 
		  glm::vec3* _vertices, 
		  int triangleCount, 
		  glm::ivec3* triangles,
		  float edgeKs, 
		  float edgeKd, 
		  float bendKs, 
		  float bendKd)
	{
		vertexCount = _vertexCount;
		vertices = _vertices;
		std::map<clothEdge, clothPair> mBendPairs;
		for (int i = 0; i < triangleCount; i ++){
			clothSpring s;
			s.kd = edgeKd; s.ks = edgeKs;
			s.type = EDGE;
			// Edge 1
			s.p1 = triangles[i].x; s.p2 = triangles[i].y;
			if (mBendPairs.find(clothEdge(s.p1, s.p2)) == mBendPairs.end()){
				s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
				springs.push_back(s);
			}
			// Edge 2
			s.p1 = triangles[i].y; s.p2 = triangles[i].z;
			if (mBendPairs.find(clothEdge(s.p1, s.p2)) == mBendPairs.end()){
				s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
				springs.push_back(s);
			}
			// Edge 3
			s.p1 = triangles[i].z; s.p2 = triangles[i].x;
			if (mBendPairs.find(clothEdge(s.p1, s.p2)) == mBendPairs.end()){
				s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
				springs.push_back(s);
			}

			clothEdge edge1(triangles[i].x, triangles[i].y);
			mBendPairs[edge1].add(triangles[i].z);

			clothEdge edge2(triangles[i].y, triangles[i].z);
			mBendPairs[edge2].add(triangles[i].x);

			clothEdge edge3(triangles[i].z, triangles[i].x);
			mBendPairs[edge1].add(triangles[i].y);
		}

		std::map<clothEdge, clothPair>::iterator iter;
		for (iter = mBendPairs.begin(); iter != mBendPairs.end(); iter++){
			if (iter->second.isValid()){
				clothSpring s;
				s.kd = bendKd; s.ks = bendKs;
				s.type = BEND;
				s.p1 = iter->second.p1; s.p2 = iter->second.p2;
				s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
				springs.push_back(s);
			}
		}
	}
	
	void updateSprings(float ks, float kd, springType type)
	{
		for (int i = 0; i < springs.size(); ++i){
			if (springs[i].type == type){
				springs[i].kd = kd;
				springs[i].ks = ks;
			}
		}
	}

	std::vector<clothSpring> springs;
	int vertexCount;
	glm::vec3* vertices;
};


#endif