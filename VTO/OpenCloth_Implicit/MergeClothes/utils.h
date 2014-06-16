#ifndef UTILS_H
#define UTILS_H

#include <opencv2/opencv.hpp>
#include <gl/glut.h>
using namespace cv;

const Vec3d	V3dZero(0.0, 0.0, 0.0);
const Matx33d I33d = Matx33d::eye();

#define MIN_VEC3(a, b, m) \
	m[0] = a[0] < b[0] ? a[0] : b[0]; \
	m[1] = a[1] < b[1] ? a[1] : b[1]; \
	m[2] = a[2] < b[2] ? a[2] : b[2];

#define MAX_VEC3(a, b, m) \
	m[0] = a[0] > b[0] ? a[0] : b[0]; \
	m[1] = a[1] > b[1] ? a[1] : b[1]; \
	m[2] = a[2] > b[2] ? a[2] : b[2];

#define EPS						1e-3

#define DIM			3
#define g			Vec3d(0.0, -9.8, 0.0)
#define mg			Vec3d(0.0, -0.0098, 0.0)

const double dt = 0.005;

const double mass = 0.01;
const double H = 0.005, h = 0.002;
const double thickness = 0.01;
const double g_cw = 0.58;
const double g_ch = 0.7;

const double ksStruct = 0.5, kdStruct = -0.25;
const double ksShear = 0.5, kdShear = -0.25;
const double ksBend = 0.85, kdBend = -0.25;
const double uf = 0.6;
const double rsl = 0.1;
const double airDamping = -0.01;

#endif