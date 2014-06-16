#include "collision.h"

using namespace collision;

Plane::Plane( Vec3d p0, Vec3d p1, Vec3d p2, double u_, double e_ )
{
	corner[0] = p0;
	corner[1] = p1;
	corner[2] = p2;
	u = u_;
	e = e_;
	computeNormal();
}

void Plane::computeNormal()
{
	Vec3d e0 = corner[1] - corner[0];
	Vec3d e1 = corner[2] - corner[0];

	Vec3d n = e0.cross(e1);
	normal =  n / sqrt(n.ddot(n));
}

Vec3d Plane::rayIntersectionPoint( Vec3d rayp, Vec3d rayd )
{
	// Ax+By+Cz+D=0
	double D = -normal.ddot(corner[0]);

	rayd = rayd / sqrt(rayd.ddot(rayd));
	double t = -(normal.ddot(rayp) + D) / normal.ddot(rayd);

	Vec3d itp = rayp + rayd * t;

	return itp;
}

bool collision::ptInTriangle( Vec3d p, Plane pl )
{
	Vec3d t0 = pl.corner[0],
		t1 = pl.corner[1],
		t2 = pl.corner[2];
	Vec3d e0 = t2 - t0;
	Vec3d e1 = t1 - t0;
	Vec3d e2 = p - t0;

	double dot00 = e0.ddot(e0);
	double dot01 = e0.ddot(e1);
	double dot02 = e0.ddot(e2);
	double dot11 = e1.ddot(e1);
	double dot12 = e1.ddot(e2);
	double inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);
	double _u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (_u < 0 || _u > 1)
	{
		return false;
	}
	double _v = (dot00 * dot12 - dot01 *dot02) * inverDeno;
	if (_v < 0 || _v > 1)
	{
		return false;
	}

	return _u + _v <= 1;
}

bool collision::collided( Particle pt, Plane pl, 
	double H, double h, bool isTriangle )
{
	Vec3d n = pl.normal;
	Vec3d p0 = pl.corner[0];

	Vec3d e = pt.cp - p0;

	if (e.dot(n) <= H + h)
	{
		if (!isTriangle)
		{
			return true;
		}
		else
		{
			Vec3d itp = pl.rayIntersectionPoint(pt.cp, pt.v);
			if (ptInTriangle(itp, pl))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

void collision::intersect( Particle &pt, Plane pl, 
	double H, double h)
{
	Vec3d v = pt.v;
	Vec3d p = pt.cp;

	Vec3d ppl = p - pl.corner[0];
	double d = ppl.ddot(pl.normal);

	if (d < H)
	{
		Vec3d itp = pl.rayIntersectionPoint(p, pl.normal);
// 		Vec3d vDir = v / sqrt(v.ddot(v));
// 		pt.cp = itp - vDir * thickness;
		pt.cp = itp + pl.normal * (H + h);
	}

	// compute normal and tangential components of velocity
	double mVn = v.ddot(pl.normal);
	Vec3d vn = mVn * pl.normal;
	Vec3d vt = v - vn;

	// new velocity = (1 - u)vt - e vn
	Vec3d nv = (1 - pl.u) * vt - pl.e * vn;
	pt.v = nv;
}

void collision::collisionPlane( ParticleSystem &ps, 
	Plane pl, double H, double h )
{
	for (size_t i = 0; i < ps.pts.size(); i++)
	{
		if (collided(ps.pts[i], pl, H, h))
		{
			intersect(ps.pts[i], pl, H, h);
		}
	}
}

void collision::traverseAABBTree( AABBNode *node0, AABBNode *node1, 
	Mesh *m0, Mesh *m1, int &cnt, double H, double h )
{
	if (!node1->intersect(node0))
	{
		return;
	}

	if (node0->vertexCount == 3)
	{
		if (node1->vertexCount == 3)
		{
			Vec3i idx0 = node0->getIndices();
			Vec3i idx1 = node1->getIndices();

			Plane pl(m1->ps.pts[idx1[0]].cp,
				m1->ps.pts[idx1[1]].cp, 
				m1->ps.pts[idx1[2]].cp);

			for (int i = 0; i < 3; i++)
			{
				int idx = idx0[i];
				if (!m0->ps.pts[idx].isPinned)
				{
					if (collided(m0->ps.pts[idx], pl, H, h, true))
					{
						cnt++;
						intersect(m0->ps.pts[idx], pl, H, h);
						//m0->ps.pts[idx].isPinned = true;
					}
				}
			}
		}
		else
		{
			if (node1->left != NULL)
			{
				traverseAABBTree(node0, node1->left, m0, m1, cnt, H, h);
			}
			if (node1->right != NULL)
			{
				traverseAABBTree(node0, node1->right, m0, m1, cnt, H, h);
			}
		}
	}
	else
	{
		if (node0->left != NULL)
		{
			traverseAABBTree(node0->left, node1, m0, m1, cnt, H, h);
		}
		if (node0->right != NULL)
		{
			traverseAABBTree(node0->right, node1, m0, m1, cnt, H, h);
		}
	}
}

void collision::collisionObject( Mesh *m0, Mesh *m1, double H, double h )
{
	int cnt = 0, clNum = 0;
	do
	{
		cnt++;
		clNum = 0;
		traverseAABBTree(m0->bboxTree, m1->bboxTree, m0, m1, clNum, H, h);
		m0->updateAABBTree();
		m1->updateAABBTree();
	}while (clNum > 0 && cnt <= 20);
}

bool collision::intersectEdge( Vec3d a0, Vec3d a1, Vec3d p0, Vec3d p1, Vec3d &ip )
{
	Vec3d a0p0 = p0 - a0;
	Vec3d a0p1 = p1 - a0;
	Vec3d a1p0 = p0 - a1;
	Vec3d a1p1 = p1 - a1;

	Vec3d a0p01 = a0p0.cross(a0p1);
	Vec3d a1p01 = a1p0.cross(a1p1);
	if (a0p01.ddot(a1p01) < 0)
	{
		Vec3d p0a01 = a0p0.cross(a1p0);
		Vec3d p1a01 = a0p1.cross(a1p1);
		if (p0a01.ddot(p1a01) < 0)
		{
			Vec3d dp = p1 - p0;
			dp /= sqrt(dp.ddot(dp));

			Vec3d a0p01 = a0p0 + dp.ddot(-a0p0) * dp;
			Vec3d a1p01 = a1p0 + dp.ddot(-a1p0) * dp;

			double da0p01 = sqrt(a0p01.ddot(a0p01));
			double da1p01 = sqrt(a1p01.ddot(a1p01));

			ip = da1p01 / (da0p01 + da1p01) * a0 + 
				da0p01 / (da0p01 + da1p01) * a1;

			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

void collision::collisionClothObject( Cloth *cl, Mesh *m )
{
	bool *visited = new bool[m->getFacesCount()];

	for (size_t i = 0; i < cl->ps.pts.size(); i++)
	{
		Vec3d p = cl->ps.pts[i].cp;
		memset(visited, 0, m->getFacesCount() * sizeof(bool));

		int randNum = rand() % m->getFacesCount();

		TriangleFace *face = &(m->getFace(randNum));
		double d;
		Plane pl;
		while (face != NULL && !visited[face->index])
		{
			visited[face->index] = true;
			Vec3d p0 = m->ps.pts[face->v[0]].cp,
				p1 = m->ps.pts[face->v[1]].cp,
				p2 = m->ps.pts[face->v[2]].cp;
			pl = Plane(p0, p1, p2);
			Vec3d itp = pl.rayIntersectionPoint(p, pl.normal);
			if (ptInTriangle(itp, pl))
			{
				d = (p - itp).ddot(pl.normal);
				break;
			}

			Vec3d bc = (p0 + p1 + p2) / 3;
			Vec3d ip;
			if (intersectEdge(itp, bc, p0, p1, ip))
			{
				face = face->neighbor[0];
			}
			else if (intersectEdge(itp, bc, p1, p2, ip))
			{
				face = face->neighbor[1];
			}
			else
			{
				intersectEdge(itp, bc, p0, p2, ip);
				face = face->neighbor[2];
			}
			Vec3d dp = p - ip;
			d = dp.ddot(dp);
		}

		if (d < H + h)
		{
			Vec3d v = cl->ps.pts[i].v;
			double mVn = v.ddot(pl.normal);
			Vec3d vn = mVn * pl.normal;
			Vec3d vt = v - vn;

			cl->ps.pts[i].v = (1 - pl.u) * vt - pl.e * vn;

			if (d < H)
			{
				Vec3d itp = pl.rayIntersectionPoint(p, v);
				Vec3d vDir = v / sqrt(v.ddot(v));
				cl->ps.pts[i].cp = itp - vDir * (H + h);
			}
		}
	}

	delete[] visited;
}
