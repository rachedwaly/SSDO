#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Material
{
public:
	glm::vec3 albedo;
	float shininess;
	float kd;
	float ks;
	float roughness;

	Material(glm::vec3 alb, float s, float kd_,float ks_,float al) {
		albedo = alb;
		shininess = s;
		kd = kd_;
		ks = ks_;
		roughness = al;
	}
	
};

