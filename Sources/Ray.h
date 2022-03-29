#pragma once

#include <iostream>
#include <memory>
#include<vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>


using namespace glm;
using namespace std;



class Ray {
public:
	Ray() {
		w = vec3(0.0);
		o = vec3(0.0);
	}
	
	Ray(vec3 origin, vec3 direction) {
		w = normalize(direction);
		o = origin;
	};
	
	bool rayTriangleIntersection(const glm::vec3& p0,const glm::vec3& p1,const glm::vec3& p2,vector<float>&result,bool culling=true) {
		const vec3 e0 = p1 - p0;
		const vec3 e1 = p2 - p0;
		const vec3 n = normalize(cross(e0, e1));
		const vec3 q = cross(w, e1);
		float a = dot(e0, q);
	
		if ((dot(n, w) >= 0) || (abs(a) < 0.0000001)) {

			return false;
		}
		


		const vec3 s = (o - p0) / a;
		const vec3 r = cross(s, e0);
		float b0 = dot(s, q);
		float b1 = dot(r, w);
		float b2 = 1 - b0 - b1;
		if ((b0 < 0) || (b1 < 0) || (b2 < 0)) {
			return false; 
		}

		float t = dot(e1, r);

		if (t >= 0) {
			//return vector<float>{b0, b1, b2, t};
			result.push_back(b2);
			result.push_back(b0);
			result.push_back(b1);
			result.push_back(t);
			return true;
		}
		return false;

	};

	vec3 w;
	vec3 o;


	
};