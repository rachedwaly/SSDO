// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Image.h"
#include "Scene.h"
#include "AABBBuilder.h"

using namespace glm;
using namespace std;

class RayTracer {
public:
	
	RayTracer();
	virtual ~RayTracer();

	inline void setResolution (int width, int height) { m_imagePtr = make_shared<Image> (width, height); }
	inline std::shared_ptr<Image> image () { return m_imagePtr; }
	void init (const std::shared_ptr<Scene> scenePtr);
	void render (const std::shared_ptr<Scene> scenePtr);
	vec3 rayTrace(vec3 x,vec3 weights, uvec4 index, const std::shared_ptr<Scene> scenePtr);
	float normalDistrubtion(vec3 n, vec3 wh, float alpha);
	vec3 fresnelTerm(vec3 wi, vec3 wh);
	float GGx(vec3 w, vec3 n, float alpha);

	void renderWithAABB(const std::shared_ptr<Scene> scenePtr);
	vec3 rayTraceAABB(const vec3 & x, const vec3& weights, const uvec4& index, const std::shared_ptr<Scene> scenePtr);
	vec3 shade(const vec3& x, const vec3& weights, const uvec4& index, shared_ptr<LightSource> light, const std::shared_ptr<Scene> scenePtr);
private:
	std::shared_ptr<Image> m_imagePtr;
	AABBBuilder* builder=0;
	

};






