#include "largevector.h"

#include <assert.h>

LargeVector<Vec3d> operator*(const LargeVector<Matx33d> other, const LargeVector<Vec3d> v) 
{
	assert(other.v.size() == v.v.size());
	LargeVector<Vec3d> tmp(v);
	for (size_t i = 0; i < v.v.size(); i++)
	{
		tmp.v[i] = Vec3d(Mat(other.v[i] * Matx31d(v.v[i])));
	}
	return tmp;
}

LargeVector<Vec3d> operator*(const double f, const LargeVector<Vec3d> other)
{
	LargeVector<Vec3d> tmp(other);
	for (size_t i = 0; i < other.v.size(); i++)
	{
		tmp.v[i] = other.v[i] * f;
	}
	return tmp;
}
LargeVector<Matx33d> operator*(const double f, const LargeVector<Matx33d> other)
{
	LargeVector<Matx33d> tmp(other);
	for(size_t i = 0; i < other.v.size(); i++)
	{
		tmp.v[i] = other.v[i] * f;
	}
	return tmp;
}
LargeVector<Vec3d> operator-(const LargeVector<Vec3d> Va, const LargeVector<Vec3d> Vb)
{
	assert(Va.v.size() == Vb.v.size());
	LargeVector<Vec3d> tmp(Va);
	for (size_t i = 0; i < Va.v.size(); i++) 
	{
		tmp.v[i] = Va.v[i] - Vb.v[i];
	}
	return tmp;
}
LargeVector<Matx33d> operator-(const LargeVector<Matx33d> Va, const LargeVector<Matx33d> Vb) 
{
	assert(Va.v.size() == Vb.v.size());
	LargeVector<Matx33d> tmp(Va);
	for (size_t i = 0; i < Va.v.size(); i++) 
	{
		tmp.v[i] = Va.v[i] - Vb.v[i];
	}
	return tmp;
}

LargeVector<Vec3d> operator+(const LargeVector<Vec3d> Va, const LargeVector<Vec3d> Vb)
{
	assert(Va.v.size() == Vb.v.size());
	LargeVector<Vec3d> tmp(Va);
	for (size_t i = 0; i < Va.v.size(); i++)
	{
		tmp.v[i] = Va.v[i] + Vb.v[i];
	}
	return tmp;
}

LargeVector<Vec3d> operator/(const double f, const LargeVector<Vec3d> v) 
{
	LargeVector<Vec3d> tmp(v);
	for (size_t i = 0; i < v.v.size(); i++) 
	{
		tmp.v[i] = v.v[i] / f;
	}
	return tmp;
}

double dot(const LargeVector<Vec3d> Va, const LargeVector<Vec3d> Vb)
{
	assert(Va.v.size() == Vb.v.size());
	double sum = 0;
	for (size_t i = 0; i < Va.v.size(); i++)
	{
		sum += Va.v[i].ddot(Vb.v[i]);
	}
	return sum;
}

void SolveConjugateGradient(LargeVector<Matx33d> A, 
	LargeVector<Vec3d>& x, LargeVector<Vec3d> b, double eps, int i_max)
{
	double eps2 = eps * eps;
	int i = 0;
	LargeVector<Vec3d> r = b - A * x;
	LargeVector<Vec3d> d = r;
	LargeVector<Vec3d> q;
	double alpha_new = 0;
	double alpha = 0;
	double beta  = 0;
	double delta_old = 0;
	double delta_new = dot(r, r);
	double delta0    = delta_new;
	while (i < i_max && delta_new > eps2) 
	{
		q = A * d;
		alpha = delta_new / dot(d, q);
		x = x + alpha * d;
		r = r - alpha * q;
		delta_old = delta_new;
		delta_new = dot(r, r);
		beta = delta_new / delta_old;
		d = r + beta * d;
		i++;
	}
}

