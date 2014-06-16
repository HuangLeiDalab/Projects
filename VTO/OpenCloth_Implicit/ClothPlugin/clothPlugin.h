#ifndef _CLOTH_PLUGIN_H
#define _CLOTH_PLUGIN_H

#include "utils.h"

extern "C" {

	_DLLExport bool initCloth(int vertexCount, glm::vec3* vertices, int triangleCount, glm::ivec3* triangles);

	_DLLExport void updateCloth(float deltaT);

	_DLLExport void clearCloth();
};

#endif