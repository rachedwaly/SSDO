// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <iostream>
#include <memory>
#include<vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "Transform.h"
#include "Ray.h"
using namespace glm;
using namespace std;

/*
struct Ray {
	vec3 w;
	vec3 o;

	Ray(vec3 origin, vec3 direction) {
		w = normalize(direction);
		o = origin;
	};

	std::vector<float> rayTriangleIntersection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
		vec3 e0 = p1 - p0;
		vec3 e1 = p2 - p0;
		vec3 n = normalize(cross(e0, e1));
		vec3 q = cross(w, e1);
		float a = dot(e0, q);

		if ((dot(n, w) >= 0) || (abs(a) < 0.001)) {

			return vector<float>{};
		}

		vec3 s = (o - p0) / a;
		vec3 r = cross(s, e0);
		float b0 = dot(s, q);
		float b1 = dot(r, w);
		float b2 = 1 - b0 - b1;
		if ((b0 < 0) || (b1 < 0) || (b2 < 0)) {
			return vector<float>{};
		}

		float t = dot(e1, r);

		if (t >= 0) {
			//return vector<float>{b0, b1, b2, t};
			return vector<float>{b2, b0, b1, t};
		}
		return vector<float>{};

	};
};
*/

/// Basic camera model
class Camera : public Transform {
public:
	inline float getFoV () const { return m_fov; }
	inline void setFoV (float f) { m_fov = f; }
	inline float getAspectRatio () const { return m_aspectRatio; }
	inline void setAspectRatio (float a) { m_aspectRatio = a; }
	inline float getNear () const { return m_near; }
	inline void setNear (float n) { m_near = n; }
	inline float getFar () const { return m_far; }
	inline void setFar (float n) { m_far = n; }


	glm::vec3 getCenter() {
		return center;
	}
	/**
	 *  The view matrix is the inverse of the camera model matrix, 
	 *  so that we can express the entire world in the camera frame 
	 *  by transforming all its entities with this matrix. 
	 *  Here, it is composed after the camera transformation and the 
	 *  rotation matrix stemming from the cmaera quaternion.
	 */
	inline glm::mat4 computeViewMatrix () { 
		center = vec3(glm::mat4_cast(curQuat) * computeTransformMatrix()[3]);
		return inverse (glm::mat4_cast (curQuat) * computeTransformMatrix ()); 
		
	}
	
	/// Returns the projection matrix stemming from the camera intrinsic parameter. 
	inline glm::mat4 computeProjectionMatrix () const {	return glm::perspective (glm::radians (m_fov), m_aspectRatio, m_near, m_far); }

	inline Ray rayAt(float u, float v) {
		
		mat4 viewMat = inverse(computeViewMatrix());
		vec3 right = normalize(vec3(viewMat[0]));
		vec3 up = normalize(vec3(viewMat[1]));
		vec3 viewDir = normalize (-vec3(viewMat[2]));
		vec3 eyePos = vec3(viewMat[3]);
		
		float h =2.f* tan(glm::radians(m_fov / 2.f));

		vec3 pixelPos = 2.f*viewDir + u *m_aspectRatio* h * right + v*h * up;
		//vec3 pixelPos =viewDir + ((u - 0.5f) * m_aspectRatio * h) * right + ((1.f - v) - 0.5f) * h * up;
		return Ray(eyePos, pixelPos);

		
	}

private:
	float m_fov = 45.f; // Vertical field of view, in degrees
	float m_aspectRatio = 1.33f;//was 1 // Ratio between the width and the height of the image
	float m_near =1.f; // Distance before which geometry is excluded fromt he rasterization process
	float m_far = 5.f; // Distance after which the geometry is excluded fromt he rasterization process
	glm::quat curQuat;
	glm::quat lastQuat;
	glm::vec3 center = vec3(0.0);
};





