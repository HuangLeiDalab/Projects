#include "particlesystem.h"

using namespace ps;

void ParticleSystem::getState( LargeVector<Vec3d> &P, LargeVector<Vec3d> &V )
{
	P.resize(pts.size());
	V.resize(pts.size());
	for (size_t i = 0; i < pts.size(); i++)
	{
		P[i] = pts[i].cp;
		V[i] = pts[i].v;
	}
}

void ParticleSystem::setState( LargeVector<Vec3d> P, LargeVector<Vec3d> V, bool updateOld )
{
	assert(P.size() == pts.size() && V.size() == pts.size());
	for (size_t i = 0; i < pts.size(); i++)
	{
		if (updateOld)
		{
			pts[i].op = pts[i].cp;
		}
		pts[i].cp = P[i];
		pts[i].v = V[i];
	}
}

void ParticleSystem::clearState()
{
	for (size_t i = 0; i < pts.size(); i++)
	{
		pts[i].v = V3dZero;
		pts[i].f = V3dZero;
	}
	t = 0;
}

void ParticleSystem::clear()
{
	pts.clear();
	springs.clear();
	t = 0;
	maxSpringLen = 1.1;
	minSpringLen = 0.98;
}

void ParticleSystem::derivative( LargeVector<Vec3d> &V, LargeVector<Vec3d> &A )
{
	clearForces();
	computeForces();
	computeConstraintForces();	// constrain force function

	V.resize(pts.size());
	A.resize(pts.size());
	for (size_t i = 0; i < pts.size(); i++)
	{
		if (pts[i].isPinned)
		{
			V[i] = V3dZero;
			A[i] = V3dZero;
		}
		else
		{
			V[i] = pts[i].v;
			A[i] = pts[i].f / pts[i].m;
		}
	}
}

void ParticleSystem::derivative( LargeVector<Matx33d> &K, LargeVector<Vec3d> &V, 
	LargeVector<Vec3d> &F, LargeVector<Matx33d> &M )
{
	clearForces();				// zero the force accumulators
	computeForces(K);			// magic force function
	computeConstraintForces();	// constrain force function

	V.resize(pts.size());
	F.resize(pts.size());
	M.resize(pts.size());
	for (size_t i = 0; i < pts.size(); i++)
	{
		if (pts[i].isPinned)
		{
			V[i] = V3dZero;
			F[i] = V3dZero;
		}
		else
		{
			V[i] = pts[i].v;
			F[i] = pts[i].f;
		}
		M[i] = pts[i].m * I33d;
	}
}

void ParticleSystem::clearForces()
{
	for (size_t i = 0; i < pts.size(); i++)
	{
		pts[i].f = V3dZero;
	}
}

void ParticleSystem::computeForces()
{
	for (size_t i = 0; i < pts.size(); i++)
	{
		pts[i].f = /*pts[i].m * */mg + pts[i].fu + airDamping * pts[i].v;
	}

	for (size_t i = 0; i < springs.size(); i++)
	{
		if (springs[i].p1 == 14 || springs[i].p2 == 14)
		{
			Vec3d b(0,0,0);
		}
		int p1 = springs[i].p1;
		int p2 = springs[i].p2;
		double l0 = springs[i].l0;
		double ks = springs[i].ks;
		double kd = springs[i].kd;

		Vec3d dx = pts[p1].cp - pts[p2].cp;
		Vec3d dv = pts[p1].v - pts[p2].v;
		double d = sqrt(dx.ddot(dx));
		Vec3d dxdir = dx / d;

		Vec3d f = -ks * (d - l0) * dxdir
			+ kd * dv.ddot(dxdir) * dxdir;

		pts[p1].f += f;
		pts[p2].f -= f;
	}
}

void ParticleSystem::computeForces( LargeVector<Matx33d> &K )
{
	K.resize(pts.size());
	K.clear();

	for (size_t i = 0; i < pts.size(); i++)
	{
		pts[i].f = pts[i].m * g + pts[i].fu + airDamping * pts[i].v;
	}

	for (size_t i = 0; i < springs.size(); i++)
	{
		int p1 = springs[i].p1;
		int p2 = springs[i].p2;
		double l0 = springs[i].l0;
		double ks = springs[i].ks;
		double kd = springs[i].kd;

		Vec3d dx = pts[p1].cp - pts[p2].cp;
		Vec3d dv = pts[p1].v - pts[p2].v;
		double d2 = dx.ddot(dx);
		double d = sqrt(d2);
		Vec3d dxdir = dx / d;

		Matx31d tmpDx = Matx31d(dx);
		Matx33d tmpK = ks * (-I33d + l0 / d * 
			(I33d - 1 / d2 * tmpDx * tmpDx.t()));
		K[p1] += tmpK;
		K[p2] += tmpK;

		Vec3d f = -ks * (d - l0) * dxdir
			+ kd * dv.ddot(dxdir) * dxdir;
		
		pts[p1].f += f;
		pts[p2].f -= f;
	}
}

void ParticleSystem::computeConstraintForces()
{
}

void ParticleSystem::addSpring( int p1, int p2, 
	double ks, double kd, Spring::SpringType t )
{
	Vec3d dx = pts[p2].cp - pts[p1].cp;
	double l0 = sqrt(dx.ddot(dx));
	springs.push_back(Spring(p1, p2, l0, ks, kd, t));
}

void ParticleSystem::applyProvotDynamicInverse()
{
	for (size_t i = 0; i < springs.size(); i++)
	{
		int p1 = springs[i].p1;
		int p2 = springs[i].p2;

		Vec3d dx = pts[p1].cp - pts[p2].cp;
		double d = sqrt(dx.ddot(dx));
		dx = dx / d;

		if (d > springs[i].l0)
		{
			d -= springs[i].l0;
			d /= 2.0;
			dx *= d;
			if (pts[p1].isPinned)
			{
				pts[p2].v += dx;
			}
			else if (pts[p2].isPinned)
			{
				pts[p1].v -= dx;
			}
			else
			{
				pts[p1].v -= dx;
				pts[p2].v += dx;
			}
		}
	}
}

void ParticleSystem::constraintSpring( double dt )
{
	for (int iter = 0; iter < 20; iter++)
	{
		for (size_t i = 0; i < springs.size(); i++)
		{
			if (springs[i].type == Spring::SPRING_BEND)
			{
				continue;
			}
			int p1 = springs[i].p1;
			int p2 = springs[i].p2;
			double l0 = springs[i].l0;

			Vec3d dx = pts[p1].cp - pts[p2].cp;
			Vec3d c = (pts[p1].cp + pts[p2].cp) / 2;
			double d = sqrt(dx.ddot(dx));

			if (d > maxSpringLen * l0)
			{
				pts[p1].cp = c + dx * maxSpringLen * l0 / (d * 2);
				pts[p2].cp = c - dx * maxSpringLen * l0 / (d * 2);
			}
			if (d < minSpringLen * l0)
			{
				pts[p1].cp = c + dx * minSpringLen * l0 / (d * 2);
				pts[p2].cp = c - dx * minSpringLen * l0 / (d * 2);
			}
		}
	}

	// recalculate the velocity
	for (size_t i = 0; i < pts.size(); i++)
	{
		if (pts[i].isPinned)
		{
			pts[i].cp = pts[i].op;
		}
		pts[i].v = (pts[i].cp - pts[i].op) / dt;
	}
}
