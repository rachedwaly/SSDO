// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#define _USE_MATH_DEFINES

#include "Mesh.h"

#include "Console.h"
#include <cmath>
#include <algorithm>

using namespace std;

Mesh::~Mesh () {
	clear ();
}

void Mesh::computeBoundingSphere (glm::vec3 & center, float & radius) const {
	center = glm::vec3 (0.0);
	radius = 0.f;
	for (const auto & p : m_vertexPositions)
		center += p;
	center /= m_vertexPositions.size ();
	for (const auto & p : m_vertexPositions)
		radius = std::max (radius, distance (center, p));
}

void Mesh::recomputePerVertexNormals (bool angleBased) {
	m_vertexNormals.clear ();
	// Change the following code to compute a proper per-vertex normal
	m_vertexNormals.resize (m_vertexPositions.size (), glm::vec3 (0.0, 0.0, 0.0));
	for (auto & t : m_triangleIndices) {
		glm::vec3 e0 (m_vertexPositions[t[1]] - m_vertexPositions[t[0]]);
		glm::vec3 e1 (m_vertexPositions[t[2]] - m_vertexPositions[t[0]]);
		glm::vec3 n = normalize (cross (e0, e1));
		for (size_t i = 0; i < 3; i++)
			m_vertexNormals[t[i]] += n;
	}
	for (auto & n : m_vertexNormals)
		n = normalize (n);
}

void Mesh::clear () {
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_triangleIndices.clear ();
}

void Mesh::computeVerticesModel() {
	m_vertexNormalsModel = {};
	m_vertexPositionsModel = {};

	glm::mat4 model = computeTransformMatrix();
	glm::mat4 normalMat = transpose(inverse(model));

	for (int i = 0; i < m_vertexPositions.size(); i++) {
		glm::vec3 p = glm::vec3(model * glm::vec4(m_vertexPositions[i], 1.0));
		glm::vec3 n= glm::vec3(normalMat * glm::vec4(m_vertexNormals[i], 1.0));
		m_vertexPositionsModel.push_back(p);
		m_vertexNormalsModel.push_back(n);
	}
	return;
}

void Mesh::calculateTextCoords() {
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	for (auto& v : m_vertexPositions) {
		minX = std::min(minX, v.x);
		maxX = std::max(maxX, v.x);
		minY = std::min(minX, v.y);
		maxY = std::max(maxY, v.y);
	}

	for (auto& v : m_vertexPositions) {
		m_textCoords.push_back(glm::vec2((v.x - minX) / (maxX - minX), (v.y - minY) / (maxY - minY)));
		
	}
	
}