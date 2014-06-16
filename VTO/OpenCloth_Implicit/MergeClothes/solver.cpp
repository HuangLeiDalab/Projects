#include "solver.h"

void EulerSolver::solve( ParticleSystem &p, double deltaT )
{
	LargeVector<Vec3d> tempP, tempV1, tempV2, tempA;

	p.derivative(tempV1, tempA);			// get deriv
	p.getState(tempP, tempV2);				// get state
	for (size_t i = 0; i < p.pts.size(); i++)
	{
		tempP[i] = tempP[i] + deltaT * tempV1[i];
		tempV2[i] = tempV2[i] + deltaT * tempA[i];
	}
	p.setState(tempP, tempV2);				// update state
	p.t += deltaT;
}

void MidpointSolver::solve( ParticleSystem &p, double deltaT )
{
	LargeVector<Vec3d> tempP1, tempP2, tempV1, tempV2, tempA;

	p.derivative(tempV2, tempA);			// get deriv
	p.getState(tempP1, tempV1);				// get state
	tempP2.resize(p.pts.size());
	for (size_t i = 0; i < p.pts.size(); i++)
	{
		tempP2[i] = tempP1[i] + deltaT * tempV2[i] / 2;
		tempV2[i] = tempV1[i] + deltaT * tempA[i] / 2;
	}

	p.setState(tempP2, tempV2);				// update state
	p.derivative(tempV2, tempA);			// get deriv at midpoint

	for (size_t i = 0; i < p.pts.size(); i++)
	{
		tempP2[i] = tempP1[i] + deltaT * tempV2[i];
		tempV2[i] = tempV1[i] + deltaT * tempA[i];
	}

	p.setState(tempP2, tempV2, false);
	p.t += deltaT;
}

void ImplicitEulerSolver::solve( ParticleSystem &p, double deltaT )
{
	LargeVector<Matx33d> K, M;
	LargeVector<Vec3d> V, F;

	p.derivative(K, V, F, M);

	LargeVector<Vec3d> V_new;
	V_new.resize(p.pts.size());

	LargeVector<Matx33d> A = M - deltaT * deltaT * K;
	LargeVector<Vec3d> b = M * V + deltaT * F;

	SolveConjugateGradient(A, V_new, b);

	for (size_t i = 0; i < p.pts.size(); i++)
	{
		p.pts[i].op = p.pts[i].cp;
		p.pts[i].cp += deltaT * V_new[i];
		p.pts[i].v = V_new[i];
	}
}
