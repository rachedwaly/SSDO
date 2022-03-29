// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2022 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------
#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Material.h"
#include "Transform.h"

class Mesh : public Transform {
public:
	virtual ~Mesh ();

	inline const std::vector<glm::vec3> & vertexPositions () const { return m_vertexPositions; } 
	inline std::vector<glm::vec3> & vertexPositions () { return m_vertexPositions; }
	inline const std::vector<glm::vec3> & vertexNormals () const { return m_vertexNormals; } 
	inline std::vector<glm::vec3> & vertexNormals () { return m_vertexNormals; } 


	inline const std::vector<glm::vec3>& vertexPositionsModel() const { return m_vertexPositionsModel; }
	inline std::vector<glm::vec3>& vertexPositionsModel() { return m_vertexPositionsModel; }
	inline const std::vector<glm::vec3>& vertexNormalsModel() const { return m_vertexNormalsModel; }
	inline std::vector<glm::vec3>& vertexNormalsModel() { return m_vertexNormalsModel; }

	void computeVerticesModel();

	inline const std::vector<glm::uvec3> & triangleIndices () const { return m_triangleIndices; }
	inline std::vector<glm::uvec3> & triangleIndices () { return m_triangleIndices; }

	/// Compute the parameters of a sphere which bounds the mesh
	void computeBoundingSphere (glm::vec3 & center, float & radius) const;
	
	void recomputePerVertexNormals (bool angleBased = false);

	void clear ();

	bool hasTexture() {
		return textured;
	}


	void setMaterial(std::shared_ptr<Material> mat) {
		textured = false;
		m_material = mat;
	};

	std::shared_ptr<Material> getMaterial() {
		return m_material;
	};

	const std::vector<glm::vec2>& textCoords() {
		if (m_textCoords.size() == 0) {
			calculateTextCoords();
		}
		return m_textCoords;
	};

private:
	std::vector<glm::vec3> m_vertexPositions;
	std::vector<glm::vec3> m_vertexPositionsModel;
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec3> m_vertexNormalsModel;
	std::vector<glm::uvec3> m_triangleIndices;
	std::vector<glm::vec2> m_textCoords;
	bool textured = true;

	std::shared_ptr<Material> m_material;

	void calculateTextCoords();

};
