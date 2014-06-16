#ifndef _CLOTH_H_
#define _CLOTH_H_

#include "utils.h"
#include <vector>
#include <map>

typedef struct _CLOTH_EDGE_{
	int e1, e2;
	_CLOTH_EDGE_(const int _e1, const int _e2){
		if (_e1 < _e2){
			e1 = _e1;
			e2 = _e2;
		}else{
			e1 = _e2;
			e2 = _e1;
		}
	}
	bool operator < (const _CLOTH_EDGE_& e) const {
		if (e1 == e.e1)
			return e2 < e.e2;
		else
			return e1 < e.e1;
	}
} clothEdge;

typedef struct _CLOTH_PAIR_{
	int p1, p2;
	_CLOTH_PAIR_() { p1 = -1; p2 = -1; }
	void add(const int v)
	{ 
		if (p1 != -1)
			p2 = v;
		else
			p1 = v;
	}
	bool isValid() { return p1 != -1 && p2 != -1; }
} clothPair;

enum springType {
	EDGE,
	BEND
};

typedef struct _CLOTH_SPRING_{
	int p1, p2;
	scalarType restLen;
	scalarType ks, kd;
	springType type;
} clothSpring;

class Cloth
{
public:
	Cloth(vec3Array& vertices, std::vector<unsigned int> &indices, const scalarType edgeKs, const scalarType edgeKd, const scalarType bendKs, const scalarType bendKd);
	~Cloth(void);
	const std::vector<clothSpring>& getSprings() const { return mSprings; }

private:
	void updateSprings(const scalarType ks, const scalarType kd, springType type);

private:
	vec3Array& mVertices;
	std::vector<unsigned int>& mIndices;
	std::vector<clothSpring> mSprings;
};

#endif