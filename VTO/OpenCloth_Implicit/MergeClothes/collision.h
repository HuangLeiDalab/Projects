#ifndef COLLISION_H
#define COLLISION_H

#include "utils.h"

#include "mesh.h"

using namespace ps;

namespace collision
{
	class Plane
	{
	public:
		double u, e;	// friction and resilience
		Vec3d corner[3];
		Vec3d normal;

		Plane(Vec3d p0 = V3dZero, Vec3d p1 = V3dZero, 
			Vec3d p2 = V3dZero, double u_ = uf, double e_ = rsl);

		void computeNormal();
		Vec3d rayIntersectionPoint(Vec3d rayp, Vec3d rayd);
	};

	bool ptInTriangle(Vec3d p, Plane pl);

	bool collided(Particle pt, Plane pl,
		double H, double h, bool isTriangle = false);

	void intersect(Particle &pt, Plane pl, double H, double h);

	void collisionPlane(ParticleSystem &ps, 
		Plane pl, double H, double h);

	void traverseAABBTree(AABBNode *node0, AABBNode *node1,
		Mesh *m0, Mesh *m1, int &cnt, double H, double h);

	void collisionObject(Mesh *m0, Mesh *m1, double H, double h);

	bool intersectEdge(Vec3d a0, Vec3d a1, Vec3d p0, Vec3d p1, Vec3d &ip);

	void collisionClothObject(Cloth *cl, Mesh *m);
}

#endif // COLLISION_H
