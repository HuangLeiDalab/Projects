#ifndef SOLVER_H
#define SOLVER_H

#include <particlesystem.h>

using namespace ps;

class Solver
{
public:
	virtual void solve(ParticleSystem &p, double deltaT) = 0;
};

class EulerSolver : public Solver
{
public:
	EulerSolver(){}
	EulerSolver(ParticleSystem &p, double deltaT){ solve(p, deltaT); }

	void solve(ParticleSystem &p, double deltaT);
};

class MidpointSolver : public Solver
{
public:
	MidpointSolver(){}
	MidpointSolver(ParticleSystem &p, double deltaT){ solve(p, deltaT); }

	void solve(ParticleSystem &p, double deltaT);
};

class ImplicitEulerSolver : public Solver
{
public:
	ImplicitEulerSolver(){}
	ImplicitEulerSolver(ParticleSystem &p, double deltaT){ solve(p, deltaT); }

	void solve(ParticleSystem &p, double deltaT);
};

#endif // SOLVER_H
