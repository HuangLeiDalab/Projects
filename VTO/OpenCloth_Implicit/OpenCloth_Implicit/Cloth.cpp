#include "Cloth.h"


Cloth::Cloth(vec3Array& vertices, std::vector<unsigned int> &indices, const scalarType edgeKs, const scalarType edgeKd, const scalarType bendKs, const scalarType bendKd)
	: mVertices(vertices), mIndices(indices)
{
	std::map<clothEdge, clothPair> mBendPairs;
	for (int i = 0; i < indices.size(); i += 3){
		clothSpring s;
		s.kd = edgeKd; s.ks = edgeKs;
		s.type = EDGE;
		// Edge 1
		s.p1 = indices[i]; s.p2 = indices[i + 1];
		if (mBendPairs.find(clothEdge(s.p1, s.p2)) == mBendPairs.end()){
			s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
			mSprings.push_back(s);
		}
		// Edge 2
		s.p1 = indices[i + 1]; s.p2 = indices[i + 2];
		if (mBendPairs.find(clothEdge(s.p1, s.p2)) == mBendPairs.end()){
			s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
			mSprings.push_back(s);
		}
		// Edge 3
		s.p1 = indices[i + 2]; s.p2 = indices[i];
		if (mBendPairs.find(clothEdge(s.p1, s.p2)) == mBendPairs.end()){
			s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
			mSprings.push_back(s);
		}
		
		clothEdge edge1(indices[i], indices[i + 1]);
		mBendPairs[edge1].add(indices[i + 2]);

		clothEdge edge2(indices[i + 1], indices[i + 2]);
		mBendPairs[edge2].add(indices[i]);

		clothEdge edge3(indices[i + 2], indices[i]);
		mBendPairs[edge1].add(indices[i + 1]);
	}

	std::map<clothEdge, clothPair>::iterator iter;
	for (iter = mBendPairs.begin(); iter != mBendPairs.end(); iter++){
		if (iter->second.isValid()){
			clothSpring s;
			s.kd = bendKd; s.ks = bendKs;
			s.type = BEND;
			s.p1 = iter->second.p1; s.p2 = iter->second.p2;
			s.restLen = glm::length(vertices[s.p1] - vertices[s.p2]);
			mSprings.push_back(s);
		}
	}
}


Cloth::~Cloth(void)
{
}

void Cloth::updateSprings(const scalarType ks, const scalarType kd, springType type)
{
	for (int i = 0; i < mSprings.size(); ++i){
		if (mSprings[i].type == type){
			mSprings[i].kd = kd;
			mSprings[i].ks = ks;
		}
	}
}