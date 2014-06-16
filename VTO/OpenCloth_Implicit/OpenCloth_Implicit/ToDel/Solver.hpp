#ifndef _SOLVER_CLOTH_H_
#define _SOLVER_CLOTH_H_

#include "mesh.hpp"
#include "cloth.hpp"
#include "largeVector.hpp"

/* const params */
const glm::vec3 Gravity = glm::vec3(0.0f,-0.00581f,0.0f);
const float DEFAULT_DAMPING = -0.125f;

const float EPS = 1e-6;
const int iteratorMax = 8;
/*******************************************************/


HashSpatial* meshCollider;

glm::vec3* X;
int VertexCount;
std::vector<glm::vec3> PreX;
std::vector<glm::vec3> V;
std::vector<glm::vec3> F;
std::vector<glm::vec3> dc_dp; //  dc/dp
std::vector<glm::mat3> df_dx; //  df/dp
std::vector<glm::mat3> df_dv; //  df/dv
std::vector<float> C; //for implicit integration
std::vector<float> C_Dot; //for implicit integration
std::vector<glm::vec3> deltaP2;

std::vector<glm::vec3> dV;
std::vector<glm::mat3> A;
glm::mat3 M = glm::mat3(1.0f);
std::vector<glm::vec3> b;
std::vector<glm::vec3> P_;
std::vector<glm::vec3> P_inv;
std::vector<float> inv_len;

std::vector<clothSpring> Springs;
int SpringCount;

void initalParams(HashSpatial* mesh, Cloth* cloth)
{
	meshCollider = mesh;
	X = cloth->vertices;
	VertexCount = cloth->vertexCount;
	Springs = cloth->springs;
	SpringCount = Springs.size();

	PreX.resize(VertexCount);
	V.resize(VertexCount);
	memset(&(V[0].x), 0, VertexCount * sizeof(glm::vec3));
	F.resize(VertexCount);
	A.resize(VertexCount);
	b.resize(VertexCount);
	dV.resize(VertexCount);
	P_.resize(VertexCount);
	memset(&(P_[0].x), 0, VertexCount * sizeof(glm::vec3));
	P_inv.resize(VertexCount);
	memset(&(P_inv[0].x), 0, VertexCount * sizeof(glm::vec3));

	C.resize(SpringCount);
	inv_len.resize(SpringCount);
	C_Dot.resize(SpringCount);
	dc_dp.resize(SpringCount);
	deltaP2.resize(SpringCount);
	df_dx.resize(SpringCount);
	df_dv.resize(SpringCount);
	memset(&(C[0]), 0, SpringCount * sizeof(float));
	memset(&(C_Dot[0]), 0, SpringCount * sizeof(float));
	memset(&(deltaP2[0].x), 0, SpringCount * sizeof(glm::vec3));
}

void clearParams()
{
	V.clear();
	F.clear();
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

void computeForces()
{
	for(int i = 0; i < VertexCount; i++) {
		F[i] = glm::vec3(0);
		F[i] += Gravity;
		//add force due to damping of velocity
		F[i] += DEFAULT_DAMPING * V[i];
	}
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 deltaP;
	glm::vec3 deltaV;
	for(int i = 0; i < SpringCount;i++) {
		p1 = X[Springs[i].p1];
		p2 = X[Springs[i].p2];
		v1 = V[Springs[i].p1];
		v2 = V[Springs[i].p2];
		deltaP = p1 - p2;
		deltaV = v1 - v2;
		float dist = glm::length(deltaP);
		inv_len[i] = 1.0f / dist;
		C[i] = dist - Springs[i].restLen;
		dc_dp[i] = deltaP / dist;
		C_Dot[i] = glm::dot(v1, -dc_dp[i]) + glm::dot(v2, dc_dp[i]);
		deltaP2[i] = glm::vec3(deltaP.x * deltaP.x, deltaP.y * deltaP.y, deltaP.z * deltaP.z);

		float leftTerm = -Springs[i].ks * (dist - Springs[i].restLen);
		float rightTerm = Springs[i].kd * (glm::dot(deltaV, deltaP) / dist);
		glm::vec3 springForce = (leftTerm + rightTerm) * glm::normalize(deltaP);

		//if(springs[i].p1 != 0 && springs[i].p1 != numX)
		F[Springs[i].p1] += springForce;
		//if(springs[i].p2 != 0 && springs[i].p2 != numX)
		F[Springs[i].p2] -= springForce;
	}
}

void calcForceDerivatives() {
	//clear the derivatives
	memset(&(df_dx[0]), 0, VertexCount * sizeof(glm::mat3));
	memset(&(df_dv[0]), 0, VertexCount * sizeof(glm::mat3));

	glm::mat3 d2C_dp2[2][2]={glm::mat3(1.0f),glm::mat3(1.0f),glm::mat3(1.0f),glm::mat3(1.0f)};
	glm::mat3 dp1;
	glm::mat3 dp2;
	glm::mat3 dp3;

	for(int i = 0; i < SpringCount; i++) {
		float c1 = C[i];
		d2C_dp2[0][0][0][0] = -c1 * deltaP2[i].x + c1;
		d2C_dp2[0][0][1][1] = -c1 * deltaP2[i].y + c1;
		d2C_dp2[0][0][2][2] = -c1 * deltaP2[i].z + c1;

		d2C_dp2[0][1][0][0] = c1 * deltaP2[i].x - c1;
		d2C_dp2[0][1][1][1] = c1 * deltaP2[i].y - c1;
		d2C_dp2[0][1][2][2] = c1 * deltaP2[i].z - c1;

		d2C_dp2[1][0]  = d2C_dp2[0][1];
		d2C_dp2[1][1]  = d2C_dp2[0][0];

		dp1  = glm::outerProduct(dc_dp[i], dc_dp[i]);
		dp2  = glm::outerProduct(dc_dp[i], -dc_dp[i]);
		dp3  = glm::outerProduct(-dc_dp[i],-dc_dp[i]);

		df_dx[i] += -Springs[i].ks * (dp1 + (d2C_dp2[0][0] * C[i]) ) - Springs[i].kd * (d2C_dp2[0][0] * C_Dot[i]);
		df_dx[i] += -Springs[i].ks * (dp2 + (d2C_dp2[0][1] * C[i]) ) - Springs[i].kd * (d2C_dp2[0][1] * C_Dot[i]);
		df_dx[i] += -Springs[i].ks * (dp2 + (d2C_dp2[1][1] * C[i]) ) - Springs[i].kd * (d2C_dp2[1][1] * C_Dot[i]);

		df_dv[i] += -Springs[i].kd * dp1;
		df_dv[i] += -Springs[i].kd * dp2;
		df_dv[i] += -Springs[i].kd * dp3;
	}
}

void SolveConjugateGradientPreconditioned(std::vector<glm::mat3> _A, std::vector<glm::vec3>& x, std::vector<glm::vec3> _b,std::vector<glm::vec3> _P, std::vector<glm::vec3> _P_inv)
{
	std::vector<glm::vec3> r = x;
	multiply(_A, r);
	sub(_b, r);
	std::vector<glm::vec3> d = r;
	multiply(_P_inv, d);
	float alpha_new = 0;
	float alpha = 0;
	float beta  = 0;
	float delta_old = 0;
	std::vector<glm::vec3> p = r;
	multiply(_P, p);
	float delta_new = dot(r, p);
	float delta0    = delta_new;
	std::vector<glm::vec3> q;
	std::vector<glm::vec3> dNew;
	int i = 0;
	while (i < iteratorMax && delta_new > EPS * delta0) {
		q = d;
		multiply(_A, q);
		alpha = delta_new / dot(d, q);
		dNew = d;
		multiply(alpha, dNew);
		add(dNew, x);
		multiply(alpha, q);
		sub(r, q);
		r = q;
		delta_old = delta_new;
		delta_new = dot(r, r);
		beta = delta_new / delta_old;
		multiply(beta, d);
		add(r, d);
		i++;
	}
}

void integrateImplicit(float deltaTime) 
{
	calcForceDerivatives();
    float y = 0.0;//correction term

	for (int i = 0; i < VertexCount; i++) {
		A[i] =  M - deltaTime * (df_dv[i] + deltaTime * df_dx[i]);
		b[i] = deltaTime * (F[i] + deltaTime * df_dx[i] * (V[i] + y));
		P_[i] = glm::vec3(A[i][0][0], A[i][1][1], A[i][2][2]);
		P_inv[i] = 1.0f / P_[i];// glm::vec3(1.0f/A[0][0], 1.0f/A[1][1], 1.0f/A[2][2]);
	}

	SolveConjugateGradientPreconditioned(A, dV, b, P_, P_inv);

	for(int i = 0; i < VertexCount; i++) {
		PreX[i] = X[i];
		V[i] += dV[i] * deltaTime;
		X[i] += deltaTime * V[i];
		if (meshCollider != NULL){
			meshCollider->intersect(&PreX[i], &X[i], &V[i]);
		}
	}
}

void applyProvotDynamicInverse()
{
	glm::vec3 p1;
	glm::vec3 p2;
	glm::vec3 deltaP;
	for(int i = 0; i < SpringCount; i++) {
		//check the current lengths of all springs
		p1 = X[Springs[i].p1];
		p2 = X[Springs[i].p2];
		deltaP = p1 - p2;
		float dist = glm::length(deltaP);
		if (dist > Springs[i].restLen * 1.01f) {
			dist -= Springs[i].restLen * 1.01f;
			dist /= 2.0f;
			deltaP = glm::normalize(deltaP) * dist;			
			V[Springs[i].p1] -= deltaP;
			V[Springs[i].p2] += deltaP;
		}
	}
}

void stepPhysics(float dt) 
{
	computeForces();
	integrateImplicit(dt);
    applyProvotDynamicInverse();
}

#endif