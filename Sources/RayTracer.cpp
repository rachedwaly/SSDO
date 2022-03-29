// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#include "RayTracer.h"

#include "Console.h"
#include "Camera.h"
#include <glm/gtx/string_cast.hpp>
RayTracer::RayTracer() :
	m_imagePtr(std::make_shared<Image>()) {}



RayTracer::~RayTracer() {
	delete(builder);
}





void RayTracer::init(const std::shared_ptr<Scene> scenePtr) {

	std::chrono::high_resolution_clock clock;

	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	Console::print("Calculating vertices in Model Space");
	for (int i = 0; i < scenePtr->numOfMeshes(); i++) {
		scenePtr->mesh(i)->computeVerticesModel();
	}
	Console::print("Calculating vertices in Model Space done");
	Console::print("Start building ");
	builder = new AABBBuilder(scenePtr);
	builder->startBuild();
	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("build done in " + std::to_string(elapsedTime) + "ms");

}

void RayTracer::render(const std::shared_ptr<Scene> scenePtr) {

	size_t width = m_imagePtr->width();
	size_t height = m_imagePtr->height();
	std::chrono::high_resolution_clock clock;
	Console::print("Start ray tracing at " + std::to_string(width) + "x" + std::to_string(height) + " resolution...");
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	m_imagePtr->clear(scenePtr->backgroundColor());

	// <---- Ray tracing code ---->

	shared_ptr<Camera> myCam = scenePtr->camera();

	vec3 c = vec3(inverse(myCam->computeViewMatrix())[3]);

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {


			//orthographic projection
			//vec3 test = vec3(vec4(x, y,0.0, 1.0));
			//Ray ray(test, vec3(0.0, 0.0, -1.0));


			Ray ray = myCam->rayAt(-1 + (2.f * x) / (width - 1), -1 + (2.f * y) / (height - 1));

			float e = 100000.0;
			for (int i = 0; i < scenePtr->numOfMeshes(); i++) {
				std::shared_ptr<Mesh> mesh = scenePtr->mesh(i);
				std::vector<glm::uvec3> myTriangleIndices = mesh->triangleIndices();
				std::vector<glm::vec3> myVertexPositions = mesh->vertexPositionsModel();

				mat4 model = mesh->computeTransformMatrix();

				for (int j = 0; j < myTriangleIndices.size(); j++) {


					vec3 p0 = vec3(myVertexPositions[myTriangleIndices[j][0]]);
					vec3 p1 = vec3(myVertexPositions[myTriangleIndices[j][1]]);
					vec3 p2 = vec3(myVertexPositions[myTriangleIndices[j][2]]);

					vector<float> inter = {};

					if (ray.rayTriangleIntersection(p0, p1, p2, inter)) {
						vec3 inter_point = ray.o + inter[3] * ray.w;
						float d = length(c - inter_point);
						if (d < e) {
							e = d;
							m_imagePtr->operator()(x, y) = rayTrace(inter_point, vec3(inter[0], inter[1], inter[2]), uvec4(myTriangleIndices[j],i), scenePtr);// I should add the normals here
						}

					}
				}
			}

		}

	}


	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Ray tracing executed in " + std::to_string(elapsedTime) + "ms");

}


vec3 RayTracer::rayTrace(vec3 x, vec3 weights, uvec4 index, const std::shared_ptr<Scene> scenePtr) {

	shared_ptr<Camera> myCam = scenePtr->camera();
	vec3 c = vec3(inverse(myCam->computeViewMatrix())[3]);
	int occlusion = 1;
	for (int i = 0; i < scenePtr->numOfLights(); i++) {
		shared_ptr<LightSource> light = scenePtr->light(i);
		if (light->isDirectional()) {
			vec3 directionOfLight = light->getDirection();

			Ray ray(x, -directionOfLight);

			for (int i = 0; i < scenePtr->numOfMeshes(); i++) {
				std::shared_ptr<Mesh> mesh = scenePtr->mesh(i);
				std::shared_ptr<Material> material = scenePtr->material(i);
				std::vector<glm::uvec3> myTriangleIndices = mesh->triangleIndices();
				std::vector<glm::vec3> myVertexPositions = mesh->vertexPositionsModel();
				std::vector<glm::vec3> myVertexNormals = mesh->vertexNormalsModel();

				

				for (int j = 0; j < myTriangleIndices.size(); j++) {


					vec3 p0 = vec3(myVertexPositions[myTriangleIndices[j][0]]);
					vec3 p1 = vec3(myVertexPositions[myTriangleIndices[j][1]]);
					vec3 p2 = vec3(myVertexPositions[myTriangleIndices[j][2]]);


					vector<float> inter = {};

					if (ray.rayTriangleIntersection(p0, p1, p2, inter)) {
						occlusion++;
					}

				}
			}
			return shade(x, weights, index, light, scenePtr) / (float)occlusion;

		}

		//if the light is positional
		else {



		}

	}

	return vec3(0.0);
}




vec3 RayTracer::fresnelTerm(vec3 wi, vec3 wh) {
	vec3 F0 = vec3(0.02);
	float dot = pow(1 - glm::max(0.0f, glm::dot(wi, wh)), 5);
	return F0 + (vec3(1.0) - F0) * dot;
}

float RayTracer::normalDistrubtion(vec3 n, vec3 wh, float alpha) {
	float a = 3.14f * pow(1 + (pow(alpha, 2) - 1) * (pow(glm::max(dot(n, wh), 0.0f), 2)), 2);
	float res = pow(alpha, 2) / a;
	return res;
}

float RayTracer::GGx(vec3 w, vec3 n, float alpha) {

	float k = alpha * sqrt(2 / 3.14);
	float temp = glm::max(dot(n, w), 0.0f);
	return temp / (k + (1 - k) * temp);

}


void RayTracer::renderWithAABB(const std::shared_ptr<Scene> scenePtr) {

	size_t width = m_imagePtr->width();
	size_t height = m_imagePtr->height();
	std::chrono::high_resolution_clock clock;
	Console::print("Start ray tracing  AABB at " + std::to_string(width) + "x" + std::to_string(height) + " resolution...");
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	m_imagePtr->clear(scenePtr->backgroundColor());

	// <---- Ray tracing code ---->

	shared_ptr<Camera> myCam = scenePtr->camera();
	vec3 c = vec3(inverse(myCam->computeViewMatrix())[3]);
	float stepx = (2.f) / (width - 1);
	float stepy = (2.f) / (height - 1);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			Ray ray = myCam->rayAt(-1 + stepx * x, -1 + stepy * y);
			float e = 100000.0;
			vector<uvec4> relevantTriangles = {};
			builder->rayTrace(ray, builder->getFirstRoot(), relevantTriangles);

			for (uvec4 index : relevantTriangles) {

				const vector<vec3>& myVertexPositions = scenePtr->mesh(index[3])->vertexPositionsModel();
				const vec3& p0 = myVertexPositions[index[0]];
				const vec3& p1 = myVertexPositions[index[1]];
				const vec3& p2 = myVertexPositions[index[2]];
				vector<float> inter = { };
				
				if (ray.rayTriangleIntersection(p0, p1, p2, inter)) {
					
					vec3 inter_point = ray.o + inter[3] * ray.w;
					float d = length(c - inter_point);
					if (d < e) {
						e = d;
						m_imagePtr->operator()(x, y) = rayTraceAABB(inter_point, vec3(inter[0], inter[1], inter[2]), index, scenePtr);// I should add the normals here
					}
				}
			}

		}

	}




	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Ray tracing executed in " + std::to_string(elapsedTime) + "ms");
}


vec3 RayTracer::rayTraceAABB(const vec3& x, const vec3& weights, const  uvec4& index, const std::shared_ptr<Scene> scenePtr) {
	vec3 colorResponse = vec3(0.0);
	int occlusion = 1;
	const float numOfLights = scenePtr->numOfLights();
	Ray ray(vec3(0.0),vec3(0.0));
	for (int i = 0; i < numOfLights; i++) {
		shared_ptr<LightSource> light = scenePtr->light(i);
		if (light->isDirectional()) {
			vec3 directionOfLight = light->getDirection();
			ray.o = x+0.1f*directionOfLight;
			ray.w = -directionOfLight;
		}
		else {
			vec3 positionOfLight = light->getDirection();
			ray.o = x;
			ray.w = positionOfLight - x;

		}
		vector<uvec4> relevantTriangles = {};
		builder->rayTrace(ray, builder->getFirstRoot(), relevantTriangles);

		for (uvec4 index_inter : relevantTriangles) {

			vector<vec3>& myVertexPositions = scenePtr->mesh(index_inter[3])->vertexPositionsModel();
			vector<vec3>& myVertexNormals = scenePtr->mesh(index_inter[3])->vertexNormalsModel();
			const vec3& p0 = myVertexPositions[index_inter[0]];
			const vec3& p1 = myVertexPositions[index_inter[1]];
			const vec3& p2 = myVertexPositions[index_inter[2]];
			vector<float> inter = {};
			if (ray.rayTriangleIntersection(p0, p1, p2, inter)) {
				occlusion=2;
			}


		}
		colorResponse +=shade(x, weights, index, light, scenePtr) / (float)occlusion;
	}
	//std::cout << to_string((colorResponse)/(float)numOfLights) << std::endl;
	colorResponse /= (float)numOfLights;
	return colorResponse;


}

vec3 RayTracer::shade(const vec3& x, const vec3& weights, const uvec4& index, shared_ptr<LightSource> light, const std::shared_ptr<Scene> scenePtr) {
	vector<vec3>& myVertexNormals = scenePtr->mesh(index[3])->vertexNormalsModel();

	std::shared_ptr<Material> material = scenePtr->material(index[3]);
	shared_ptr<Camera> myCam = scenePtr->camera();
	const vec3& n0 = myVertexNormals[index[0]];
	const vec3& n1 = myVertexNormals[index[1]];
	const vec3& n2 = myVertexNormals[index[2]];

	vec3 n = normalize(n0 * weights[0] + n1 * weights[1] + n2 * weights[2]);

	vec3 wi = normalize(-light->getDirection());
	vec3 wo = normalize(myCam->getCenter() - x);
	vec3 wh = normalize(wi + wo);

	vec3 fsi = normalDistrubtion(n, wh, material->roughness) * fresnelTerm(wi, wh) * GGx(wi, n, material->roughness) * GGx(wo, n, material->roughness);

	float dotProduct = glm::max(dot(n, wh) * dot(n, wi), 0.1f);



	vec3 fs = light->getColor() * material->albedo * fsi / (4 * dotProduct)*glm::max(dot(n,wi),0.f);


	vec3 fd = (material->kd / 3.14f) * light->getColor() * material->albedo * glm::max(dot(n, wi), 0.f);

	vec3 f = fd + fs;

	return light->getIntensity() * f;
}

/*
void RayTracer::renderWithAABB(const std::shared_ptr<Scene> scenePtr) {

	size_t width = m_imagePtr->width();
	size_t height = m_imagePtr->height();
	std::chrono::high_resolution_clock clock;
	Console::print("Start ray tracing  AABB at " + std::to_string(width) + "x" + std::to_string(height) + " resolution...");
	std::chrono::time_point<std::chrono::high_resolution_clock> before = clock.now();
	m_imagePtr->clear(scenePtr->backgroundColor());

	// <---- Ray tracing code ---->

	shared_ptr<Camera> myCam = scenePtr->camera();
	vec3 c = vec3(inverse(myCam->computeViewMatrix())[3]);

	float stepx = ((2.f) / (width - 1));
	float stepy = ((2.f) / (height - 1));

	for (size_t x = 0; x < width; x++) {
		for (size_t y = 0; y < height; y++) {
			vec3 colorResponse = vec3(0.0);
			for (int i = 0; i <2; i++) {
				for (int j = 0; j <2; j++) {
					float rx = ((double)rand() / (RAND_MAX)) *stepx ;
					float ry = ((double)rand() / (RAND_MAX)) *stepy;

					//std::cout << "////////" << std::endl;
					//std::cout << rx << std::endl;
					//std::cout << ry << std::endl;
					//std::cout << (2.f ) / (width - 1) << std::endl;
					//std::cout << "////////" << std::endl;
					Ray ray = myCam->rayAt(-1 + (stepx*x)+rx, -1 + (stepy*y)+ry);
					float e = 100000.0;
					vector<uvec4> relevantTriangles = {};
					builder->rayTrace(ray, builder->getFirstRoot(), relevantTriangles);
					for (uvec4 index : relevantTriangles) {

						const vector<vec3>& myVertexPositions = scenePtr->mesh(index[3])->vertexPositionsModel();
						vec3 p0 = myVertexPositions[index[0]];
						vec3 p1 = myVertexPositions[index[1]];
						vec3 p2 = myVertexPositions[index[2]];
						vector<float> inter = { };
						if (ray.rayTriangleIntersection(p0, p1, p2, inter)) {

							vec3 inter_point = ray.o + inter[3] * ray.w;
							float d = length(c - inter_point);
							if (d < e) {
								e = d;
								colorResponse += shadeAABB(inter_point, vec3(inter[0], inter[1], inter[2]), index, scenePtr);

							}
						}
					}
				}
				if (length(colorResponse) > 0) {
					m_imagePtr->operator()(x, y) = colorResponse/4.f;
				}



			}

		}

	}




	std::chrono::time_point<std::chrono::high_resolution_clock> after = clock.now();
	double elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
	Console::print("Ray tracing executed in " + std::to_string(elapsedTime) + "ms");
}
*/

//Ray ray = myCam->rayAt(-1 + (2.f * x) / (width - 1), -1 + (2.f * y) / (height - 1));
//
//float e = 100000.0;
//vector<uvec4> relevantTriangles = {};
//builder->rayTrace(ray, builder->getFirstRoot(), relevantTriangles);
//
//
//
//for (uvec4 index : relevantTriangles) {
//
//	vector<vec3>& myVertexPositions = scenePtr->mesh(index[3])->vertexPositions();
//
//	vec3 p0 = myVertexPositions[index[0]];
//	vec3 p1 = myVertexPositions[index[1]];
//	vec3 p2 = myVertexPositions[index[2]];
//
//
//	vector<float> inter = { };
//
//	if (ray.rayTriangleIntersection(p0, p1, p2, inter)) {
//
//		vec3 inter_point = ray.o + inter[3] * ray.w;
//		float d = length(c - inter_point);
//		if (d < e) {
//			e = d;
//
//			m_imagePtr->operator()(x, y) = vec3(1.0, 0.0, 0.0);
//
//			//m_imagePtr->operator()(x, y) = shadeAABB(inter_point, vec3(inter[0], inter[1], inter[2]), index, scenePtr);// I should add the normals here
//		}
//
//	}
//}



