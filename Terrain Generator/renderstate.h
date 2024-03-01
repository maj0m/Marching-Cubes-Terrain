#pragma once
#include "framework.h"
#include "material.h"
#include "light.h"

struct RenderState {
	float time;
	vec3 wEye;
	mat4 MVP, M, V, P;
	Material* material;
	std::vector<Light> lights;
};
