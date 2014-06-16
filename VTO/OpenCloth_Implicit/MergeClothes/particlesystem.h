#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "largevector.h"

#include <fstream>
#include <QString>

namespace ps
{
	struct Particle
	{
		double	m;	// mass
		Vec3d	cp;	// current position
		Vec3d	op;	// old position
		Vec3d	v;	// velocity
		Vec3d	f;	// force accumulator
		Vec3b	c;	// color
		Vec3d	fp;	// penalty force
		Vec3d	fu; // user force

		bool isPinned;

		Particle()
		{
			m	= 0.0;
			cp	= V3dZero;
			op	= V3dZero;
			v	= V3dZero;
			f	= V3dZero;
			c	= Vec3b(255, 255, 255);
			fp	= V3dZero;
			fu	= V3dZero;
			isPinned = false;
		}
	};

	struct Spring
	{
		int p1, p2;		// two particles' id
		double l0;		// rest length
		double ks, kd;	// coefficient

		enum SpringType{SPRING_STRUCTURAL = 0, SPRING_SHEAR, SPRING_BEND};
		SpringType type;

		Spring(){}
		Spring(int i, int j, double l0_, double ks_, double kd_, SpringType t_)
		{
			p1 = i;
			p2 = j;
			l0 = l0_;
			ks = ks_;
			kd = kd_;
			type = t_;
		}
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(){ clear(); }
		vector<Particle> pts;
		vector<Spring> springs;
		double t;					// simulation clock
		double maxSpringLen;		// 
		double minSpringLen;

		// gather state from the particles into dst
		void getState(LargeVector<Vec3d> &P, LargeVector<Vec3d> &V);

		// scatter state from src into the particles
		void setState(LargeVector<Vec3d> P, LargeVector<Vec3d> V, bool updateOld = true);

		// clear state for next run
		void clearState();

		void clear();

		// calculate derivative, place in dst
		void derivative(LargeVector<Vec3d> &V, LargeVector<Vec3d> &A);

		void derivative(LargeVector<Matx33d> &K, LargeVector<Vec3d> &V, 
			LargeVector<Vec3d> &F, LargeVector<Matx33d> &M);

		// zero the force accumulators
		void clearForces();

		void computeForces();

		void computeForces(LargeVector<Matx33d> &K);

		// compute the constraint force
		void computeConstraintForces();

		// add a spring
		void addSpring(int p1, int p2, double ks, double kd, Spring::SpringType t);

		void applyProvotDynamicInverse();

		void constraintSpring(double dt);

		void saveForce()
		{
			QString str = QString("force_%1.txt").arg(t);
			std::ofstream out(str.toStdString());
			for (size_t i = 0; i < pts.size(); i++)
			{
				out << pts[i].f << pts[i].v << pts[i].cp << endl;
			}
		}
	};
};

#endif // PARTICLE_H
