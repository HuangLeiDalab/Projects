/*
Copyright (c) 2011, Movania Muhammad Mobeen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other
materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

//A simple cloth using implicit integration based on the model by Baraff
//& Witkin in the paper "Large steps in cloth simulation" and the SIGGRAPH course notes
//"Realtime Physics" http://www.matthiasmueller.info/realtimephysics/coursenotes.pdf using
//GLUT,GLEW and GLM libraries. This code is intended for beginners so that they may
//understand what is required to implement implicit integration based cloth simulation.
//
//This code is under BSD license. If you make some improvements,
//or are using this in your research, do let me know and I would appreciate
//if you acknowledge this in your code.
//
//Controls:
//left click on any empty region to rotate, middle click to zoom
//left click and drag any point to drag it.
//
//Author: Movania Muhammad Mobeen
//        School of Computer Engineering,
//        Nanyang Technological University,
//        Singapore.
//Email : mova0002@e.ntu.edu.sg

#include "utils.h"
#include "ToDel/Solver.hpp"
#include "ToDel/subdivision.hpp"
#include <fstream>
#define SHOW_OBJ

#pragma comment(lib, "glew32.lib")

using namespace std;
const int width = 1024, height = 1024;

int numX = 50, numY=50;
size_t total_points = (numX+1)*(numY+1);
int size = 4;
float hsize = size/2.0f;

float timeStep =  10 / 60.0f;
float currentTime = 0;
double accumulator = timeStep;
int selected_index = -1;

struct Spring {
	int p1, p2;
	float rest_length;
	float Ks, Kd;
	int type;
};


char info[MAX_PATH]={0};

int oldX=0, oldY=0;
float rX=15, rY=0;
int state =1 ;
float dist=-23;
const int GRID_SIZE=10;


extern glm::vec3* X;
extern vector<glm::vec3> V;
vector<GLushort> indices;
/*
vector<GLushort> indices;
vector<Spring> springs;

vector<glm::vec3> PreX;
vector<glm::vec3> X;
vector<glm::vec3> V;
vector<glm::vec3> F;
vector<glm::vec3> dc_dp; //  dc/dp
vector<glm::mat3> df_dx; //  df/dp
vector<glm::mat3> df_dv; //  df/dv
vector<float> C; //for implicit integration
vector<float> C_Dot; //for implicit integration
vector<glm::vec3> deltaP2;

char info[MAX_PATH]={0};

int oldX=0, oldY=0;
float rX=15, rY=0;
int state =1 ;
float dist=-23;
const int GRID_SIZE=10;

const int STRUCTURAL_SPRING = 0;
const int SHEAR_SPRING = 1;
const int BEND_SPRING = 2;

int spring_count=0;

const float DEFAULT_DAMPING =  -0.125f;
float	KsStruct = 0.75f,KdStruct = -0.25f;
float	KsShear = 0.75f,KdShear = -0.25f;
float	KsBend = 0.95f,KdBend = -0.25f;
glm::vec3 gravity=glm::vec3(0.0f,-0.00581f,0.0f);*/
float mass =1.f;

GLint viewport[4];
GLdouble MV[16];
GLdouble P[16];

LARGE_INTEGER frequency;        // ticks per second
LARGE_INTEGER t1, t2;           // ticks
double frameTimeQP=0;
float frameTime =0 ;

glm::vec3 Up=glm::vec3(0,1,0), Right, viewDir;
float startTime =0, fps=0;
int totalFrames=0;
int totalTime =0;


//const float EPS = 0.001f;
const float EPS2 = EPS*EPS;
const int i_max = 10;

glm::mat4 ellipsoid, inverse_ellipsoid;
int iStacks = 30;
int iSlices = 30;
float fRadius = 1;

// Resolve constraint in object space
glm::vec3 center = glm::vec3(0,0,0); //object space center of ellipsoid
float radius = 1;					 //object space radius of ellipsoid

/*
void StepPhysics(float dt );
void SolveConjugateGradient2(glm::mat2 A, glm::vec2& x, glm::vec2 b) {
	float i =0;
	glm::vec2 r = b - A*x;
	glm::vec2 d = r;
	glm::vec2 q = glm::vec2(0);
	float alpha_new = 0;
	float alpha = 0;
	float beta  = 0;
	float delta_old = 0;
	float delta_new = glm::dot(r,r);
	float delta0    = delta_new;
	while(i<i_max && delta_new> EPS2*delta0) {
		q = A*d;
		alpha = delta_new/glm::dot(d,q);
		x = x + alpha*d;
		r = r - alpha*q;
		delta_old = delta_new;
		delta_new = glm::dot(r,r);
		beta = delta_new/delta_old;
		d = r + beta*d;
		i++;
	}
}


template<class T>
class LargeVector {
private:
	vector<T> v;

public:

	LargeVector() {

	}
	LargeVector(const LargeVector& other) {
		v.resize(other.v.size());
		memcpy(&v[0], &(other.v[0]), sizeof(other.v[0])*other.v.size());
	}
	void resize(const int size) {
		v.resize(size);
	}
	void clear(bool isIdentity=false) {
		memset(&v[0], 0, sizeof(T)*v.size());
		if(isIdentity) {
			for(size_t i=0;i<v.size();i++) {
				v[i] = T(1);
			}
		}
	}


	T& operator[](int index) {
		return v[index];
	}
	friend LargeVector<glm::vec3> operator*(const LargeVector<glm::mat3> other, const LargeVector<glm::vec3> f );
	friend LargeVector<glm::vec3> operator*(const float f, const LargeVector<glm::vec3> other);
	friend LargeVector<glm::vec3> operator-(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb );
	friend LargeVector<glm::vec3> operator*(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb );
	friend LargeVector<glm::vec3> operator+(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb );

	friend LargeVector<glm::mat3> operator*(const float f, const LargeVector<glm::mat3> other);
	friend LargeVector<glm::mat3> operator-(const LargeVector<glm::mat3> Va, const LargeVector<glm::mat3> Vb );


	friend LargeVector<glm::vec3> operator/(const float f, const LargeVector<glm::vec3> v );
	friend float dot(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb );
};

LargeVector<glm::vec3> operator*(const LargeVector<glm::mat3> other, const LargeVector<glm::vec3> v ) {
	LargeVector<glm::vec3> tmp(v);
	for(size_t i=0;i<v.v.size();i++) {
		tmp.v[i] = other.v[i] * v.v[i];
	}
	return tmp;
}

LargeVector<glm::vec3> operator*(const LargeVector<glm::vec3> other, const LargeVector<glm::vec3> v ) {
	LargeVector<glm::vec3> tmp(v);
	for(size_t i=0;i<v.v.size();i++) {
		tmp.v[i] = other.v[i] * v.v[i];
	}
	return tmp;
}


LargeVector<glm::vec3> operator*(const float f, const LargeVector<glm::vec3> other) {
	LargeVector<glm::vec3> tmp(other);
	for(size_t i=0;i<other.v.size();i++) {
		tmp.v[i] = other.v[i]*f;
	}
	return tmp;
}
LargeVector<glm::mat3> operator*(const float f, const LargeVector<glm::mat3> other) {
	LargeVector<glm::mat3> tmp(other);
	for(size_t i=0;i<other.v.size();i++) {
		tmp.v[i] = other.v[i]*f;
	}
	return tmp;
}
LargeVector<glm::vec3> operator-(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb ) {
	LargeVector<glm::vec3> tmp(Va);
	for(size_t i=0;i<Va.v.size();i++) {
		tmp.v[i] = Va.v[i] - Vb.v[i];
	}
	return tmp;
}
LargeVector<glm::mat3> operator-(const LargeVector<glm::mat3> Va, const LargeVector<glm::mat3> Vb ) {
	LargeVector<glm::mat3> tmp(Va);
	for(size_t i=0;i<Va.v.size();i++) {
		tmp.v[i] = Va.v[i] - Vb.v[i];
	}
	return tmp;
}

LargeVector<glm::vec3> operator+(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb ) {
	LargeVector<glm::vec3> tmp(Va);
	for(size_t i=0;i<Va.v.size();i++) {
		tmp.v[i] = Va.v[i] + Vb.v[i];
	}
	return tmp;
}

LargeVector<glm::vec3> operator/(const float f, const LargeVector<glm::vec3> v ) {
	LargeVector<glm::vec3> tmp(v);
	for(size_t i=0;i<v.v.size();i++) {
		tmp.v[i] = v.v[i] / f;
	}
	return tmp;
}


float dot(const LargeVector<glm::vec3> Va, const LargeVector<glm::vec3> Vb ) {
	float sum = 0;
	for(size_t i=0;i<Va.v.size();i++) {
		sum += glm::dot(Va.v[i],Vb.v[i]);
	}
	return sum;
}

void SolveConjugateGradient(LargeVector<glm::mat3> A, LargeVector<glm::vec3>& x, LargeVector<glm::vec3> b) {
	float i =0;
	LargeVector<glm::vec3> r = b - A*x;
	LargeVector<glm::vec3> d = r;
	LargeVector<glm::vec3> q;
	float alpha_new = 0;
	float alpha = 0;
	float beta  = 0;
	float delta_old = 0;
	float delta_new = dot(r,r);
	float delta0    = delta_new;
	while(i<i_max && delta_new> EPS2*delta0) {
		q = A*d;
		alpha = delta_new/dot(d,q);
		x = x + alpha*d;
		r = r - alpha*q;
		delta_old = delta_new;
		delta_new = dot(r,r);
		beta = delta_new/delta_old;
		d = r + beta*d;
		i++;
	}
}




void SolveConjugateGradientPreconditioned(LargeVector<glm::mat3> A, LargeVector<glm::vec3>& x, LargeVector<glm::vec3> b,LargeVector<glm::vec3> P, LargeVector<glm::vec3> P_inv) {
	float i =0;
	LargeVector<glm::vec3> r =  (b - A*x);
	LargeVector<glm::vec3> d = P_inv*r;
	LargeVector<glm::vec3> q;
	float alpha_new = 0;
	float alpha = 0;
	float beta  = 0;
	float delta_old = 0;
	float delta_new = dot(r,P*r);
	float delta0    = delta_new;
	while(i<i_max && delta_new> EPS2*delta0) {
		q = A*d;
		alpha = delta_new/dot(d,q);
		x = x + alpha*d;
		r = r - alpha*q;
		delta_old = delta_new;
		delta_new = dot(r,r);
		beta = delta_new/delta_old;
		d = r + beta*d;
		i++;
	}
}


void SolveConjugateGradient(glm::mat3 A, glm::vec3& x, glm::vec3 b) {
	float i =0;
	glm::vec3 r = b - A*x;
	glm::vec3 d = r;
	glm::vec3 q = glm::vec3(0);
	float alpha_new = 0;
	float alpha = 0;
	float beta  = 0;
	float delta_old = 0;
	float delta_new = glm::dot(r,r);
	float delta0    = delta_new;
	while(i<i_max && delta_new> EPS2*delta0) {
		q = A*d;
		alpha = delta_new/glm::dot(d,q);
		x = x + alpha*d;
		r = r - alpha*q;
		delta_old = delta_new;
		delta_new = glm::dot(r,r);
		beta = delta_new/delta_old;
		d = r + beta*d;
		i++;
	}
}

*/

glm::vec3 testPos[3] = {glm::vec3(1.f, 0.2f, 0.f),
						glm::vec3(3.f, 0.2f, 0.f),
						glm::vec3(3.2f, 2.f, 0.f) };
glm::ivec3 testIndice(0, 1, 2);
vector<glm::vec3> objPos;
vector<glm::vec3> objNormals;
vector<glm::ivec3> objIndices;
float objScale = 0.1f;
Mesh *objMesh = NULL;
Mesh *objMesh2 = NULL;
Subdivide *divideObj = NULL;
HashSpatial *objHash = NULL;
void read_from_file(char* filename, vector<glm::vec3>& objPos, vector<glm::vec3>& objNormals, vector<unsigned int>& objIndices, float objScale)
{
	//glm::vec3 mat_color(0.6); 
	// vertices and color

	std::ifstream infile(filename);
	if(!infile.good())
	{
		printf("Error in loading file %s\n", filename);
		exit(0);
	}
	char buffer[256];
	unsigned int ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7, ip8;
	unsigned int n0, n1, n2;
	glm::vec3 pos;

	while(!infile.getline(buffer,255).eof())
	{
		buffer[255] = '\0';
		if(buffer[0] == 'v' && (buffer[1] == ' ' || buffer[1] == 32))
		{
			if(sscanf_s(buffer, "v %f %f %f", &pos.x, &pos.y, &pos.z) == 3)
			{
				pos = objScale * pos;
				objPos.push_back(pos);
			}
			else
			{
				printf("Vertex is not in desired format.\n");
				exit(0);
			}
		}
		else if (buffer[0] == 'v' && buffer[1] == 'n' && (buffer[2] == ' ' || buffer[2] == 32))
		{
			// load normals from obj file.
		}
		else if (buffer[0] == 'f' && (buffer[1] == ' ' || buffer[1] == 32))
		{
			if(sscanf_s(buffer, "f %u %u %u", &ip0, &ip1, &ip2) == 3)
			{
				objIndices.push_back(--ip0);
				objIndices.push_back(--ip1);
				objIndices.push_back(--ip2);
			}
			else if(sscanf_s(buffer, "f %u//%u %u//%u %u//%u", &ip0, &n0, &ip1, &n1, &ip2, &n2) == 6)
			{
				objIndices.push_back(--ip0);
				objIndices.push_back(--ip1);
				objIndices.push_back(--ip2);
			}
			else if(sscanf_s(buffer, "f %u/%u %u/%u %u/%u", &ip0, &n0, &ip1, &n1, &ip2, &n2) == 6)
			{
				objIndices.push_back(--ip0);
				objIndices.push_back(--ip1);
				objIndices.push_back(--ip2);
			}
			else if(sscanf_s(buffer, "f %u/%u/%u %u/%u/%u %u/%u/%u", &ip0, &ip1, &ip2, &ip3, &ip4, &ip5, &ip6, &ip7, &ip8) == 9)
			{
				objIndices.push_back(--ip0);
				objIndices.push_back(--ip3);
				objIndices.push_back(--ip6);
			}
			else
			{
				printf("Triangle indices is not in desired format.\n");
				exit(0);
			}
		}
	}
	// normals

	unsigned int id, size;
	bool vert_norm = (objNormals.size() != objPos.size());
	if(vert_norm)
		objNormals.resize(objPos.size(), glm::vec3(0.0f));

	size = objIndices.size();
	glm::ivec3 triangle;
	glm::vec3 p0, p1, p2;
	glm::vec3 norm;
	float phi0, phi1, phi2;
	float pi = glm::radians(180.0f);
	for(id = 0; id < size; id+=3)
	{
		triangle = glm::ivec3(objIndices[id], objIndices[id+1], objIndices[id+2]);
		p0 = objPos[triangle.x];
		p1 = objPos[triangle.y];
		p2 = objPos[triangle.z];
		norm = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		// add by HL @ 2014/3/17
		//m_tri_normal.push_back(norm);
		// end HL

		// calculate vertex normal
		if(vert_norm)
		{
			phi0 = std::acos(glm::dot(p1 - p0, p2 - p0) / (glm::length(p1 - p0) * glm::length(p2 - p0)));
			phi1 = std::acos(glm::dot(p0 - p1, p2 - p1) / (glm::length(p0 - p1) * glm::length(p2 - p1)));
			phi2 = pi - phi0 - phi1;

			objNormals[triangle.x] += phi0 * norm;
			objNormals[triangle.y] += phi1 * norm;
			objNormals[triangle.z] += phi2 * norm;
		}
	}
	// re-normalize all normals
	for(std::vector<glm::vec3>::iterator iter = objNormals.begin(); iter != objNormals.end(); ++iter)
	{
		*iter = glm::normalize(*iter);
		//m_colors.push_back(mat_color);
		//m_colors.push_back(*iter);
	}
	
}

void read_from_file(char* filename, vector<glm::vec3>& objPos, vector<glm::vec3>& objNormals, vector<glm::ivec3>& objIndices, float objScale)
{
	//glm::vec3 mat_color(0.6); 
	// vertices and color

	std::ifstream infile(filename);
	if(!infile.good())
	{
		printf("Error in loading file %s\n", filename);
		exit(0);
	}
	char buffer[256];
	unsigned int ip0, ip1, ip2, ip3, ip4, ip5, ip6, ip7, ip8;
	unsigned int n0, n1, n2;
	glm::vec3 pos;

	while(!infile.getline(buffer,255).eof())
	{
		buffer[255] = '\0';
		if(buffer[0] == 'v' && (buffer[1] == ' ' || buffer[1] == 32))
		{
			if(sscanf_s(buffer, "v %f %f %f", &pos.x, &pos.y, &pos.z) == 3)
			{
				pos = objScale * pos;
				objPos.push_back(pos);
			}
			else
			{
				printf("Vertex is not in desired format.\n");
				exit(0);
			}
		}
		else if (buffer[0] == 'v' && buffer[1] == 'n' && (buffer[2] == ' ' || buffer[2] == 32))
		{
			// load normals from obj file.
		}
		else if (buffer[0] == 'f' && (buffer[1] == ' ' || buffer[1] == 32))
		{
			if(sscanf_s(buffer, "f %u %u %u", &ip0, &ip1, &ip2) == 3)
			{
			
				objIndices.push_back(glm::ivec3(--ip0, --ip1, --ip2));
			}
			else if(sscanf_s(buffer, "f %u//%u %u//%u %u//%u", &ip0, &n0, &ip1, &n1, &ip2, &n2) == 6)
			{
				objIndices.push_back(glm::ivec3(--ip0, --ip1, --ip2));
			}
			else if(sscanf_s(buffer, "f %u/%u %u/%u %u/%u", &ip0, &n0, &ip1, &n1, &ip2, &n2) == 6)
			{
				objIndices.push_back(glm::ivec3(--ip0, --ip1, --ip2));
			}
			else if(sscanf_s(buffer, "f %u/%u/%u %u/%u/%u %u/%u/%u", &ip0, &ip1, &ip2, &ip3, &ip4, &ip5, &ip6, &ip7, &ip8) == 9)
			{
			 
				objIndices.push_back(glm::ivec3(--ip0, --ip3, --ip6));
			}
			else
			{
				printf("Triangle indices is not in desired format.\n");
				exit(0);
			}
		}
	}
	// normals

	unsigned int id, size;
	bool vert_norm = (objNormals.size() != objPos.size());
	if(vert_norm)
		objNormals.resize(objPos.size(), glm::vec3(0.0f));

	size = objIndices.size();
	glm::ivec3 triangle;
	glm::vec3 p0, p1, p2;
	glm::vec3 norm;
	float phi0, phi1, phi2;
	float pi = glm::radians(180.0f);
	for(id = 0; id < size; id++)
	{
		triangle = objIndices[id];
		p0 = objPos[triangle.x];
		p1 = objPos[triangle.y];
		p2 = objPos[triangle.z];
		norm = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		// add by HL @ 2014/3/17
		//m_tri_normal.push_back(norm);
		// end HL

		// calculate vertex normal
		if(vert_norm)
		{
			phi0 = std::acos(glm::dot(p1 - p0, p2 - p0) / (glm::length(p1 - p0) * glm::length(p2 - p0)));
			phi1 = std::acos(glm::dot(p0 - p1, p2 - p1) / (glm::length(p0 - p1) * glm::length(p2 - p1)));
			phi2 = pi - phi0 - phi1;

			objNormals[triangle.x] += phi0 * norm;
			objNormals[triangle.y] += phi1 * norm;
			objNormals[triangle.z] += phi2 * norm;
		}
	}
	// re-normalize all normals
	for(std::vector<glm::vec3>::iterator iter = objNormals.begin(); iter != objNormals.end(); ++iter)
	{
		*iter = glm::normalize(*iter);
		//m_colors.push_back(mat_color);
		//m_colors.push_back(*iter);
	}

}
void DrawObj()
{

	if (objMesh != NULL){
		objMesh->draw();
		//objHash->drawBBox();
		//objMesh->drawAABB(10, vec3(0.0, 1.0, 1.0));
	}
	if (objMesh2 != NULL){
		objMesh2->draw();
	}
}




/*
LargeVector<glm::vec3> dV;
LargeVector<glm::mat3> A;
glm::mat3 M = glm::mat3(1.0f);
LargeVector<glm::vec3> b;
LargeVector<glm::vec3> P_;
LargeVector<glm::vec3> P_inv;
vector<float> inv_len;



void AddSpring(int a, int b, float ks, float kd, int type) {
	Spring spring;
	spring.p1=a;
	spring.p2=b;
	spring.Ks=ks;
	spring.Kd=kd;
	spring.type = type;
	glm::vec3 deltaP = X[a]-X[b];
	spring.rest_length = sqrt(glm::dot(deltaP, deltaP));
	springs.push_back(spring);
}*/

bool stepGo = false;
void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
		int window_y = (height - y);
		float norm_y = float(window_y)/float(height/2.0);
		int window_x = x ;
		float norm_x = float(window_x)/float(width/2.0);

		float winZ=0;
		glReadPixels( x, height-y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
		if(winZ==1)
			winZ=0;
		double objX=0, objY=0, objZ=0;
		gluUnProject(window_x,window_y, winZ,  MV,  P, viewport, &objX, &objY, &objZ);
		glm::vec3 pt(objX,objY, objZ);
		size_t i=0;
		for(i=0;i<total_points;i++) {
			if( glm::distance(X[i],pt)<0.1) {
				selected_index = i;
				printf("Intersected at %d\n",i);
				break;
			}
		}
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;

	if(s==GLUT_UP) {
		// To DELET
		if (stepGo)
			stepGo = false;
		else
			stepGo = true;

		selected_index= -1;
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void OnMouseMove(int x, int y)
{
	if(selected_index == -1) {
		if (state == 0)
			dist *= (1 + (y - oldY)/60.0f);
		else
		{
			rY += (x - oldX)/5.0f;
			rX += (y - oldY)/5.0f;
		}
	} else {
		float delta = 1500/abs(dist);
		float valX = (x - oldX)/delta;
		float valY = (oldY - y)/delta;
		if(abs(valX)>abs(valY))
			glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
		else
			glutSetCursor(GLUT_CURSOR_UP_DOWN);

		V[selected_index] = glm::vec3(0);
		X[selected_index].x += Right[0]*valX ;
		float newValue = X[selected_index].y+Up[1]*valY;
		if(newValue>0)
			X[selected_index].y = newValue;
		X[selected_index].z += Right[2]*valX + Up[2]*valY;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

bool start = false;
void OnKeyDown(unsigned char key, int x, int y)
{
	switch (key){
	case 'g':
	case 'G':
		if (!start)
			start = true;
		else
			start = false;
		break;
	case 'w':
		for (int i = 0; i < total_points; i++){
			X[i].y += 0.1f;
		}
		break;
	case 's':
		for (int i = 0; i < total_points; i++){
			X[i].y -= 0.1f;
		}
		break;
	case 'a':
		for (int i = 0; i < total_points; i++){
			X[i].x -= 0.1f;
		}
		break;
	case 'd':
		for (int i = 0; i < total_points; i++){
			X[i].x += 0.1f;
		}
		break;
	case 'q':
		for (int i = 0; i < total_points; i++){
			X[i].z += 0.05f;
		}
		break;
	case 'e':
		for (int i = 0; i < total_points; i++){
			X[i].z -= 0.05f;
		}
		break;
	case 'z':
		for (int i = 0; i < objPos.size(); i++){
			objPos[i].x += 1.01f;
		}
		//objHash->updateHashTable();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void DrawGrid()
{
	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f);
	for(int i=-GRID_SIZE;i<=GRID_SIZE;i++)
	{
		glVertex3f((float)i,0,(float)-GRID_SIZE);
		glVertex3f((float)i,0,(float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE,0,(float)i);
		glVertex3f((float)GRID_SIZE,0,(float)i);
	}
	glEnd();
}

vector<glm::vec3> clothPos;
vector<glm::vec3> clothNormals;
//vector<unsigned int> clothIndices;
vector<glm::ivec3> clothIndices;

Cloth *clothMesh = NULL;

void InitGL() {
	startTime = (float)glutGet(GLUT_ELAPSED_TIME);
	currentTime = startTime;

	// get ticks per second
    QueryPerformanceFrequency(&frequency);

    // start timer
    QueryPerformanceCounter(&t1);

	glEnable(GL_DEPTH_TEST);
	int i=0, j=0, count=0;
	int l1=0, l2=0;
	float ypos = 7.0f;
	int v = numY+1;
	int u = numX+1;

	read_from_file("clothqunzi.obj", clothPos, clothNormals, clothIndices, 0.05f);
	//
	/*glm::mat4 m = glm::rotate(glm::mat4(1), -90.0f, glm::vec3(1, 0, 0));
	for (int i = 0; i < clothPos.size(); i++){
		glm::vec4 pos = m * glm::vec4(clothPos[i], 1.0);
		clothPos[i].x = pos.x;
		clothPos[i].y = pos.y;
		clothPos[i].z = pos.z;
	}*/
	clothMesh = new Cloth(clothPos.size(), &clothPos[0],  clothIndices.size(), &clothIndices[0], 0.95, -0.15, 0.25, -0.25);
	total_points = clothPos.size();
	//indices.resize( numX*numY*2*3);
	/*indices.resize(clothIndices.size());
	total_points = clothPos.size();//add by HL
	PreX.resize(total_points);
	X.resize(total_points);
	V.resize(total_points);
	F.resize(total_points);

	A.resize(total_points);
	b.resize(total_points);
	dV.resize(total_points);
	P_.resize(total_points);
	P_inv.resize(total_points);*/


	//fill in X
	/*for( j=0;j<=numY;j++) {
		for( i=0;i<=numX;i++) {
			X[count++] = glm::vec3( ((float(i)/(u-1)) *2-1)* hsize, size+1 + 0.5, ((float(j)/(v-1) )* size + 0.5));
		}
	}*/
	// update by HL
	/*for (j = 0; j < total_points; j++){
		X[j] = clothPos[j];
	}*/
	//fill in V
	//memset(&(V[0].x),0,total_points*sizeof(glm::vec3));

	//fill in indices
	/*GLushort* id=&indices[0];
	for (i = 0; i < numY; i++) {
		for (j = 0; j < numX; j++) {
			int i0 = i * (numX+1) + j;
			int i1 = i0 + 1;
			int i2 = i0 + (numX+1);
			int i3 = i2 + 1;
			if ((j+i)%2) {
				*id++ = i0; *id++ = i2; *id++ = i1;
				*id++ = i1; *id++ = i2; *id++ = i3;
			} else {
				*id++ = i0; *id++ = i2; *id++ = i3;
				*id++ = i0; *id++ = i3; *id++ = i1;
			}
		}
	}*/
	// update by HL
	for (j = 0; j < clothIndices.size(); j++){
		//indices[j] = clothIndices[j];
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glPolygonMode(GL_BACK, GL_LINE);
	glPointSize(5);

	wglSwapIntervalEXT(0);

	//setup springs
	/*// Horizontal
	for (l1 = 0; l1 < v; l1++)	// v
		for (l2 = 0; l2 < (u - 1); l2++) {
			AddSpring((l1 * u) + l2,(l1 * u) + l2 + 1,KsStruct,KdStruct,STRUCTURAL_SPRING);
		}

	// Vertical
	for (l1 = 0; l1 < (u); l1++)
		for (l2 = 0; l2 < (v - 1); l2++) {
			AddSpring((l2 * u) + l1,((l2 + 1) * u) + l1,KsStruct,KdStruct,STRUCTURAL_SPRING);
		}


	// Shearing Springs
	for (l1 = 0; l1 < (v - 1); l1++)
		for (l2 = 0; l2 < (u - 1); l2++) {
			AddSpring((l1 * u) + l2,((l1 + 1) * u) + l2 + 1,KsShear,KdShear,SHEAR_SPRING);
			AddSpring(((l1 + 1) * u) + l2,(l1 * u) + l2 + 1,KsShear,KdShear,SHEAR_SPRING);
		}


	// Bend Springs
	for (l1 = 0; l1 < (v); l1++) {
		for (l2 = 0; l2 < (u - 2); l2++) {
			AddSpring((l1 * u) + l2,(l1 * u) + l2 + 2,KsBend,KdBend,BEND_SPRING);
		}
		AddSpring((l1 * u) + (u - 3),(l1 * u) + (u - 1),KsBend,KdBend,BEND_SPRING);
	}
	for (l1 = 0; l1 < (u); l1++) {
		for (l2 = 0; l2 < (v - 2); l2++) {
			AddSpring((l2 * u) + l1,((l2 + 2) * u) + l1,KsBend,KdBend,BEND_SPRING);
		}
		AddSpring(((v - 3) * u) + l1,((v - 1) * u) + l1,KsBend,KdBend,BEND_SPRING);
	}*/
	/*clothIndices.resize(indices.size());
	for (j = 0; j < clothIndices.size(); j++){
		clothIndices[j] = indices[j];
	}
	clothMesh = new Cloth(X, clothIndices, 0.75, -0.25, 0.25, -0.25);*/
	/*const std::vector<clothSpring>& Springs = clothMesh->getSprings();
	int type;
	for (j = 0; j < Springs.size(); j++){
		if (Springs[j].type == EDGE)
			type = STRUCTURAL_SPRING;
		else
			type = BEND_SPRING;
		AddSpring(Springs[j].p1, Springs[j].p2, Springs[j].ks, Springs[j].kd, type);
	}

	int total_springs = springs.size();
	C.resize(total_springs );
	inv_len.resize(total_springs );
	C_Dot.resize(total_springs );
	dc_dp.resize(total_springs );
	deltaP2.resize(total_springs );
	df_dx.resize(total_springs );
	df_dv.resize(total_springs );
	memset(&(C[0]),0,total_springs*sizeof(float));
	memset(&(C_Dot[0]),0,total_springs*sizeof(float));
	memset(&(deltaP2[0].x),0,total_springs*sizeof(glm::vec3));

 	memset(&(P_[0].x),0,total_points*sizeof(glm::vec3));
	memset(&(P_inv[0].x),0,total_points*sizeof(glm::vec3));*/
	
	//create a basic ellipsoid object
	ellipsoid = glm::translate(glm::mat4(1),glm::vec3(0,2,0));
	ellipsoid = glm::rotate(ellipsoid, 45.0f ,glm::vec3(1,0,0));
	ellipsoid = glm::scale(ellipsoid, glm::vec3(fRadius,fRadius,fRadius/2));
	inverse_ellipsoid = glm::inverse(ellipsoid);

	read_from_file("human.obj", objPos, objNormals, objIndices, 0.1f);
	
	objMesh = new Mesh(objPos.size(), &objPos[0], objIndices.size(), &objIndices[0]);
	divideObj = new Subdivide(3, testPos, 1, &testIndice, 4);
	objMesh2 = new Mesh(divideObj->getVertexCnt(), divideObj->getVertices(), divideObj->getTriangleCnt(), divideObj->getTriangles());
	//objMesh2 = new Mesh(3, testPos, 1, &testIndice);
	objHash = new HashSpatial(objMesh);
	initalParams(objHash, clothMesh);
}

void OnReshape(int nw, int nh) {
	glViewport(0,0,nw, nh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)nw / (GLfloat)nh, 1.f, 100.0f);

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_PROJECTION_MATRIX, P);

	glMatrixMode(GL_MODELVIEW);
}

void OnRender() {
	size_t i=0;
	float newTime = (float) glutGet(GLUT_ELAPSED_TIME);
	frameTime = newTime-currentTime;
	currentTime = newTime;
	//accumulator += frameTime;

	//Using high res. counter
    QueryPerformanceCounter(&t2);
	 // compute and print the elapsed time in millisec
    frameTimeQP = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
	t1=t2;
	accumulator += frameTimeQP;

	++totalFrames;
	if((newTime-startTime)>1000)
	{
		float elapsedTime = (newTime-startTime);
		fps = (totalFrames/ elapsedTime)*1000 ;
		startTime = newTime;
		totalFrames=0;
	}

	sprintf_s(info, "FPS: %3.2f, Frame time (GLUT): %3.4f msecs, Frame time (QP): %3.3f", fps, frameTime, frameTimeQP);
	glutSetWindowTitle(info);

	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0,0,dist);
	glRotatef(rX,1,0,0);
	glRotatef(rY,0,1,0);

	glGetDoublev(GL_MODELVIEW_MATRIX, MV);
	viewDir.x = (float)-MV[2];
	viewDir.y = (float)-MV[6];
	viewDir.z = (float)-MV[10];
	Right = glm::cross(viewDir, Up);

	//draw grid
	DrawGrid();
#ifdef SHOW_OBJ
	DrawObj();
#endif


	//draw ellipsoid
	/*glColor3f(0,1,0);
	glPushMatrix();
		glMultMatrixf(glm::value_ptr(ellipsoid));
			glutWireSphere(fRadius, iSlices, iStacks);
	glPopMatrix();*/

	//draw polygons
	glColor3f(1,1,1);
	glBegin(GL_TRIANGLES);
	/*for(i=0;i<indices.size();i+=3) {
		glm::vec3 p1 = X[indices[i]];
		glm::vec3 p2 = X[indices[i+1]];
		glm::vec3 p3 = X[indices[i+2]];
		glVertex3f(p1.x,p1.y,p1.z);
		glVertex3f(p2.x,p2.y,p2.z);
		glVertex3f(p3.x,p3.y,p3.z);
	}*/
	for(i=0;i< clothIndices.size();i++) {
		glm::vec3 p1 = X[clothIndices[i].x];
		glm::vec3 p2 = X[clothIndices[i].y];
		glm::vec3 p3 = X[clothIndices[i].z];
		glVertex3f(p1.x,p1.y,p1.z);
		glVertex3f(p2.x,p2.y,p2.z);
		glVertex3f(p3.x,p3.y,p3.z);
	}
	glEnd();

	//draw points
	
	glBegin(GL_POINTS);
	for(i=0;i<total_points;i++) {
		glm::vec3 p = X[i];
		int is = (i==selected_index);
		glColor3f((float)!is,(float)is,(float)is);
		glVertex3f(p.x,p.y,p.z);
	}
	glEnd();

	glutSwapBuffers();
}

/*
void OnShutdown() {
	X.clear();
	V.clear();
	F.clear();
	indices.clear();
	springs.clear();
	dc_dp.clear();
	df_dx.clear();
	df_dv.clear();
	C.clear();
	C_Dot.clear();
	deltaP2.clear();

	dV.clear();
	A.clear();

	b.clear();
	P_.clear();
	P_inv.clear();
	inv_len.clear();
}

void ComputeForces() {
	size_t i=0;

	for(i=0;i<total_points;i++) {
		F[i] = glm::vec3(0);

		//add gravity force
		//if(i!=0 && i!=(numX)	)
			F[i] += gravity;
		//add force due to damping of velocity
		F[i] += DEFAULT_DAMPING*V[i];
	}


	for(i=0;i<springs.size();i++) {
		glm::vec3 p1 = X[springs[i].p1];
		glm::vec3 p2 = X[springs[i].p2];
		glm::vec3 v1 = V[springs[i].p1];
		glm::vec3 v2 = V[springs[i].p2];
		glm::vec3 deltaP = p1-p2;
		glm::vec3 deltaV = v1-v2;
		float dist = glm::length(deltaP);
		inv_len[i] = 1.0f/dist;
		C[i] = dist-springs[i].rest_length;
		dc_dp[i] = deltaP/dist;
		C_Dot[i] = glm::dot(v1, -dc_dp[i]) + glm::dot(v2, dc_dp[i]);
		deltaP2[i] = glm::vec3(deltaP.x*deltaP.x,deltaP.y*deltaP.y, deltaP.z*deltaP.z);

		float leftTerm = -springs[i].Ks * (dist-springs[i].rest_length);
		float rightTerm = springs[i].Kd * (glm::dot(deltaV, deltaP)/dist);
		glm::vec3 springForce = (leftTerm + rightTerm)*glm::normalize(deltaP);

		//if(springs[i].p1 != 0 && springs[i].p1 != numX)
			F[springs[i].p1] += springForce;
		//if(springs[i].p2 != 0 && springs[i].p2 != numX)
			F[springs[i].p2] -= springForce;
	}
}

void CalcForceDerivatives() {
	//clear the derivatives
	memset(&(df_dx[0]),0,total_points*sizeof(glm::mat3));
	memset(&(df_dv[0]),0,total_points*sizeof(glm::mat3));

	size_t i=0;

	glm::mat3 d2C_dp2[2][2]={glm::mat3(1.0f),glm::mat3(1.0f),glm::mat3(1.0f),glm::mat3(1.0f)};


	for(i=0;i<springs.size();i++) {
		float c1 = C[i];
		d2C_dp2[0][0][0][0] = (-c1*deltaP2[i].x+c1);
		d2C_dp2[0][0][1][1] = (-c1*deltaP2[i].y+c1);
		d2C_dp2[0][0][2][2] = (-c1*deltaP2[i].z+c1);

		d2C_dp2[0][1][0][0] = (c1*deltaP2[i].x-c1);
		d2C_dp2[0][1][1][1] = (c1*deltaP2[i].y-c1);
		d2C_dp2[0][1][2][2] = (c1*deltaP2[i].z-c1);

		d2C_dp2[1][0]  = d2C_dp2[0][1];
		d2C_dp2[1][1]  = d2C_dp2[0][0];

		glm::mat3 dp1  = glm::outerProduct(dc_dp[i], dc_dp[i]);
		glm::mat3 dp2  = glm::outerProduct(dc_dp[i], -dc_dp[i]);
		glm::mat3 dp3  = glm::outerProduct(-dc_dp[i],-dc_dp[i]);

		df_dx[i] += -springs[i].Ks* (dp1 + (d2C_dp2[0][0]*C[i]) ) - springs[i].Kd * (d2C_dp2[0][0]*C_Dot[i]);
		df_dx[i] += -springs[i].Ks* (dp2 + (d2C_dp2[0][1]*C[i]) ) - springs[i].Kd * (d2C_dp2[0][1]*C_Dot[i]);
		df_dx[i] += -springs[i].Ks* (dp2 + (d2C_dp2[1][1]*C[i]) ) - springs[i].Kd * (d2C_dp2[1][1]*C_Dot[i]);

		df_dv[i] += -springs[i].Kd*dp1;
		df_dv[i] += -springs[i].Kd*dp2;
		df_dv[i] += -springs[i].Kd*dp3;
	}
}


void IntegrateImplicit(float deltaTime) {

	float h = deltaTime;
	CalcForceDerivatives();
    float y = 0.0;//correction term
	size_t i=0;
	for(i=0;i<total_points;i++) {
		A[i] =  M - h * (df_dv[i] + h * df_dx[i]);
		b[i] = (h * (F[i] + h * df_dx[i] * (V[i] + y)));
		P_[i] = glm::vec3(A[i][0][0], A[i][1][1], A[i][2][2]);
		P_inv[i] = 1.0f/P_[i];// glm::vec3(1.0f/A[0][0], 1.0f/A[1][1], 1.0f/A[2][2]);
	}

	SolveConjugateGradientPreconditioned(A, dV, b, P_, P_inv);

	for(i=0;i<total_points;i++) {
		PreX[i] = X[i];
		V[i] += (dV[i]*deltaTime);
		X[i] += deltaTime * V[i];

		if(X[i].y <0) {
			X[i].y = 0;
		}

		objHash->intersect(PreX[i], X[i], V[i]);
		
		/*for (int j = 0; j < objIndices.size(); j += 3){
			glm::vec3 p1 = objPos[objIndices[j]];
			glm::vec3 p2 = objPos[objIndices[j+1]];
			glm::vec3 p3 = objPos[objIndices[j+2]];
			collider::collideTriangle(PreX[i], X[i], V[i], p1, p2, p3);
			
		}
	}
}

void ApplyProvotDynamicInverse() {

	for(size_t i=0;i<springs.size();i++) {
		//check the current lengths of all springs
		glm::vec3 p1 = X[springs[i].p1];
		glm::vec3 p2 = X[springs[i].p2];
		glm::vec3 deltaP = p1-p2;
		float dist = glm::length(deltaP);
		if(dist>(springs[i].rest_length*1.01f)) {
			dist -= (springs[i].rest_length*1.01f);
			dist /= 2.0f;
			deltaP = glm::normalize(deltaP);
			deltaP *= dist;
			if(springs[i].p1==0 || springs[i].p1 ==numX) {
				V[springs[i].p2] += deltaP;
			} else if(springs[i].p2==0 || springs[i].p2 ==numX) {
			 	V[springs[i].p1] -= deltaP;
			} else {
				V[springs[i].p1] -= deltaP;
				V[springs[i].p2] += deltaP;
			}
		}
	}
}

void EllipsoidCollision() {
	for(size_t i=0;i<total_points;i++) {
		glm::vec4 X_0 = (inverse_ellipsoid*glm::vec4(X[i],1));
		glm::vec3 delta0 = glm::vec3(X_0.x, X_0.y, X_0.z) - center;
		float distance = glm::length(delta0);
		if (distance < 1.0f) {
			delta0 = (radius - distance) * delta0 / distance;

			// Transform the delta back to original space
			glm::vec3 delta;
			glm::vec3 transformInv;
			transformInv = glm::vec3(ellipsoid[0].x, ellipsoid[1].x, ellipsoid[2].x);
			transformInv /= glm::dot(transformInv, transformInv);
			delta.x = glm::dot(delta0, transformInv);
			transformInv = glm::vec3(ellipsoid[0].y, ellipsoid[1].y, ellipsoid[2].y);
			transformInv /= glm::dot(transformInv, transformInv);
			delta.y = glm::dot(delta0, transformInv);
			transformInv = glm::vec3(ellipsoid[0].z, ellipsoid[1].z, ellipsoid[2].z);
			transformInv /= glm::dot(transformInv, transformInv);
			delta.z = glm::dot(delta0, transformInv);
			X[i] +=  delta ;
			V[i] = glm::vec3(0);
		} 
	}
}
*/
void OnIdle() {


	/*
	//Semi-fixed time stepping
	if ( frameTime > 0.0 )
    {
        const float deltaTime = min( frameTime, timeStep );
        StepPhysics(deltaTime );
        frameTime -= deltaTime;
    }
	*/

	//Fixed time stepping + rendering at different fps
	//if (stepGo){
	if (start){
	if ( accumulator >= timeStep )
    {
		objHash->updateHashTable();
        stepPhysics(timeStep);
        accumulator -= timeStep;
    }

	    glutPostRedisplay();
	}
	//}
}

/*
void StepPhysics(float dt ) {
	ComputeForces();

		IntegrateImplicit(dt);
		//EllipsoidCollision();
		//objMesh->updateAABBTree();
	ApplyProvotDynamicInverse();

}*/

void main(int argc, char** argv) {
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("GLUT Cloth Demo [Implicit Integration-Baraff & Witkin's Model]");

	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnReshape);
	glutIdleFunc(OnIdle);

	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
//	glutCloseFunc(OnShutdown);
	glutKeyboardFunc(OnKeyDown);

	glewInit();
	InitGL();

	glutMainLoop();
	delete objMesh;
	delete objHash;
	delete divideObj;
}
