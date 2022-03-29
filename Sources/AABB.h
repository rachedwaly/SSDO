#pragma once
#include <iostream>
#include <memory>
#include<vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>
#include "Ray.h"
#include "Scene.h"

using namespace glm;
using namespace std;

class AABB {

public:
	AABB(vec3 min, vec3 max) {
		Pmin = min;
		Pmax = max;
	};

	AABB(vec3 start, float width, float height, float depth) {
		Pmin = start;
		Pmax = start + vec3(width, height, depth);
	}

	~AABB() {
		delete(leftAABB);
		delete(rightAABB);
	}

	void inflate(float power) {
		vec3 inflate = (Pmin - Pmax) / power;
		Pmax = Pmax + inflate;
		Pmin = Pmin - inflate;

	}

	vec3 getMax() { return Pmax; };

	vec3 getMin() { return Pmin; };

	

	

	

	


	void setData(vector<uvec4> d) {
		readyForRT = true;
		data = d;
	};

	vector<uvec4> trianglesInside() {
		return data;
	}

	bool isReady() {
		return readyForRT;
	}

	vector<uvec4> getData() {
		return data;
	}

	AABB(const vector<uvec4>& triangles, const shared_ptr<Scene> scene) {
		constexpr float max = -100.0;
		constexpr float min = 100.0;
		glm::vec3 pmin = vec3(min, min, min);
		glm::vec3 pmax = vec3(max, max, max);

		for (uvec4 index : triangles) {
			for (int i = 0; i < 3; i++) {
				vec3 p = scene->mesh(index[3])->vertexPositionsModel()[index[i]];
				float x = p.x;
				float y = p.y;
				float z = p.z;

				if (x < pmin.x) {
					pmin.x = x;
				}
				if (y < pmin.y) {
					pmin.y = y;
				}
				if (z < pmin.z) {
					pmin.z = z;
				}

				if (x > pmax.x) {
					pmax.x = x;
				}
				if (y > pmax.y) {
					pmax.y = y;
				}
				if (z > pmax.z) {
					pmax.z = z;
				}
			}
		}
		Pmin = pmin;
		Pmax = pmax;
	}
	
	//useful for debugging
	AABB(vector<vec3> points) {
		constexpr float max = -std::numeric_limits<float>::max();
		constexpr float min = std::numeric_limits<float>::max();
		glm::vec3 pmin = vec3(min, min, min);
		glm::vec3 pmax = vec3(max, max, max);
		for (vec3 point : points) {

			vec3 p = point;
			float x = p.x;
			float y = p.y;
			float z = p.z;

			if (x < pmin.x) {
				pmin.x = x;
			}
			if (y < pmin.y) {
				pmin.y = y;
			}
			if (z < pmin.z) {
				pmin.z = z;
			}

			if (x > pmax.x) {
				pmax.x = x;
			}
			if (y > pmax.y) {
				pmax.y = y;
			}
			if (z > pmax.z) {
				pmax.z = z;
			}

		}
		Pmin = pmin;
		Pmax = pmax;
	}




	/*
	bool rayIntersect(const Ray& ray) { //returns a vector of size 2: parameters for entry and exit point
		float tstart = -std::numeric_limits<float>::max();
		float tend = std::numeric_limits<float>::max();

		vec3 direction = ray.w;
		vec3 origin = ray.o;
		for (int i = 0; i < 2; i++) {
			if ((direction[i] == 0) && ((origin[i] < Pmin[i]) || (origin[i] > Pmax[i]))) {
				return false;
			}
		}

		for (int i = 0; i < 2; i++) {
			if (direction[i] != 0) {
				float t1 = (Pmin[i] - origin[i]) / direction[i];
				float t2 = (Pmax[i] - origin[i]) / direction[i];
				if (t1 > t2) {
					float tmp = t1;
					t1 = t2;
					t2 = tmp;
				}
				if (t1 > tstart) {
					tstart = t1;
				}
				if (t2 < tend) {
					tend = t2;
				}
			}
		}

		if (tstart <= tend) {
			//result = vector<float>{ tstart, tend };
			return true ;
		}
		else {
			return false;
		}

	};
	*/
	/*
	bool rayIntersect(const Ray& r)
	{
		float tmin = (Pmin.x - r.o.x) / r.w.x;
		float tmax = (Pmax.x - r.o.x) / r.w.x;

		if (tmin > tmax) swap(tmin, tmax);

		float tymin = (Pmin.y - r.o.y) / r.w.y;
		float tymax = (Pmax.y - r.o.y) / r.w.y;

		if (tymin > tymax) swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		float tzmin = (Pmin.z - r.o.z) / r.w.z;
		float tzmax = (Pmax.z - r.o.z) / r.w.z;

		if (tzmin > tzmax) swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;
		
		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;
		
		return true;
	}
	*/

	bool rayIntersect(const Ray& r) {
		// r.dir is unit direction vector of ray
		
		float dirfracx = 1.0f / r.w.x;
		float dirfracy = 1.0f / r.w.y;
		float dirfracz = 1.0f / r.w.z;


		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		float t1 = (Pmin.x - r.o.x) * dirfracx;
		float t2 = (Pmax.x - r.o.x) * dirfracx;
		float t3 = (Pmin.y - r.o.y) * dirfracy;
		float t4 = (Pmax.y - r.o.y) * dirfracy;
		float t5 = (Pmin.z - r.o.z) * dirfracz;
		float t6 = (Pmax.z - r.o.z) * dirfracz;

		float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
		float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			return false;
		}

		return true;
	}
	void setLeft(AABB* box) {
		leftAABB = box;
	}

	void setRight(AABB* box) {
		rightAABB = box;
	}

	AABB* getLeft() {
		return leftAABB;
	}

	AABB* getRight() {
		return rightAABB;
	}

private:
	vec3 Pmin, Pmax;
	AABB* leftAABB = 0;
	AABB* rightAABB = 0;

	vector<uvec4> data = {}; //can be a vector to store more than one triangle by an AABB
	bool readyForRT = false;


};