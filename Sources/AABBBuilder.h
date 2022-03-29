#pragma once
#include "AABB.h"
#include <algorithm>
#include "Scene.h"
#include <glm/gtx/string_cast.hpp>
#include "Ray.h"


class AABBBuilder {
public:
	AABBBuilder(const shared_ptr<Scene> myScene ) {
		
		scene = myScene;
		
		constexpr float max = -std::numeric_limits<float>::max();
		constexpr float min = std::numeric_limits<float>::max();
		glm::vec3 pmin = vec3(min, min, min);
		glm::vec3 pmax = vec3(max, max, max);
		
		for (int i = 0; i < myScene->numOfMeshes(); i++) {
			const std::vector<glm::vec3>& myVertexPositions = (myScene->mesh(i))->vertexPositionsModel();

			for (glm::vec3 point : myVertexPositions) {
				float x = point.x;
				float y = point.y;
				float z = point.z;

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
			
			firstRoot = new AABB(pmin, pmax);
			
			
	};

	~AABBBuilder() {
		delete(firstRoot);
	}

	void startBuild();


	void build(std::vector<uvec4> in, AABB* root,int s);

	void build2(std::vector<uvec4> in, AABB* root, int s);

	AABB* getFirstRoot() { return firstRoot; };

	bool AABBBuilder::rayTrace(const Ray& ray, AABB* root, vector<uvec4>& result);
	

	

private:
	AABB* firstRoot=0;
	shared_ptr<Scene> scene=0;
	int sum = 0;
};