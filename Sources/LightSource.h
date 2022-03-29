#pragma once


#include "Transform.h"

class LightSource :public Transform
{
public:

	//constructor creates a directional light by default
	LightSource(glm::vec3 d, glm::vec3 c, float i) {
		color = c;
		direction = d;
		intensity = i;
	}


	LightSource(glm::vec3 d, glm::vec3 c, float i, bool directional) {
		color = c;
		direction = d;
		intensity = i;
		isdirectional = directional;
	}

	//direction will play the role of the position in case directional is false
    glm::vec3 getDirection() { 	
		if (isdirectional) {
			return normalize(direction);
		}
		else {
			return direction;
		}
	
	}

	glm::vec3 getColor() { return color; }

	bool isDirectional() { return isdirectional; };

	bool setDirectional(bool b) { isdirectional = b; }

	float getIntensity() { return intensity; }
private:
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
	bool isdirectional = true;

};