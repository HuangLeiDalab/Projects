#ifndef _LARGEVECTOR_H_
#define _LARGEVECTOR_H_

#include <vector>
#include "clothSimulatorGlobal.h"

void add(const std::vector<glm::vec3>& src, std::vector<glm::vec3>& dst)
{
	for (int i = 0; i < src.size(); i++){
		dst[i] = src[i] + dst[i];
	}
}

void sub (const std::vector<glm::vec3>& src, std::vector<glm::vec3>& dst)
{
	for (int i = 0; i < src.size(); i++){
		dst[i] = src[i] - dst[i];
	}
}

void sub (const std::vector<glm::mat3>& src, std::vector<glm::mat3>& dst)
{
	for (int i = 0; i < src.size(); i++){
		dst[i] = src[i] - dst[i];
	}
}

void multiply(const std::vector<glm::vec3>& src, std::vector<glm::vec3>& dst)
{
	for (int i = 0; i < src.size(); i++){
		dst[i] = src[i] * dst[i];
	}
}

void multiply(float f, std::vector<glm::vec3>& dst)
{
	for (int i = 0; i < dst.size(); i++){
		dst[i] = dst[i] * f;
	}
}

void multiply(float f, std::vector<glm::mat3>& dst)
{
	for (int i = 0; i < dst.size(); i++){
		dst[i] = dst[i] * f;
	}
}

void multiply(const std::vector<glm::mat3>& src, std::vector<glm::vec3>& dst)
{
	for (int i = 0; i < dst.size(); i++){
		dst[i] = src[i] * dst[i];
	}
}

void divide(float f, std::vector<glm::vec3>& dst)
{
	for (int i = 0; i < dst.size(); i++){
		dst[i] = dst[i] / f;
	}
}

float dot(const std::vector<glm::vec3>& src1, const std::vector<glm::vec3>& src2)
{
	float sum = 0.f;
	for(int i = 0; i < src1.size(); i++) {
		sum += glm::dot(src1[i], src2[i]);
	}
	return sum;
}



#endif 