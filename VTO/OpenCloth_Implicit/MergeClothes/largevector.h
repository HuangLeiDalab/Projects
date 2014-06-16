#ifndef LARGEVECTOR_H
#define LARGEVECTOR_H

#include "utils.h"

#include <vector>
using namespace std;

template<class T>
class LargeVector 
{
private:
	vector<T> v;

public:
	LargeVector() 
	{
	}
	LargeVector(const LargeVector& other)
	{
		v.resize(other.v.size());
		memcpy(&v[0], &(other.v[0]), sizeof(other.v[0]) * other.v.size());
	}
	void resize(const int size)
	{
		v.resize(size);
	}
	void clear(bool isIdentity = false) 
	{
		memset(&v[0], 0, sizeof(T) * v.size());
		if (isIdentity) 
		{
			for(size_t i = 0; i < v.size(); i++) 
			{
				v[i] = T(1);
			}
		}
	}
	size_t size() 
	{
		return v.size();
	}

	T& operator[](int index) 
	{
		return v[index];
	}
	friend LargeVector<Vec3d> operator*(const LargeVector<Matx33d> other, const LargeVector<Vec3d> f);
	friend LargeVector<Vec3d> operator*(const double f, const LargeVector<Vec3d> other);
	friend LargeVector<Vec3d> operator-(const LargeVector<Vec3d> Va, const LargeVector<Vec3d> Vb);
	friend LargeVector<Vec3d> operator+(const LargeVector<Vec3d> Va, const LargeVector<Vec3d> Vb);

	friend LargeVector<Matx33d> operator*(const double f, const LargeVector<Matx33d> other);
	friend LargeVector<Matx33d> operator-(const LargeVector<Matx33d> Va, const LargeVector<Matx33d> Vb);

	friend LargeVector<Vec3d> operator/(const double f, const LargeVector<Vec3d> v);
	friend double dot(const LargeVector<Vec3d> Va, const LargeVector<Vec3d> Vb);
};

void SolveConjugateGradient(LargeVector<Matx33d> A, 
	LargeVector<Vec3d>& x, LargeVector<Vec3d> b, 
	double eps = EPS, int i_max = 10);

#endif // LARGEVECTOR_H
