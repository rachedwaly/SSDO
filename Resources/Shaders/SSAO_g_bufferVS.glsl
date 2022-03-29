#version 450 core // Minimal GL version support expected from the GPU

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoords;

uniform mat4 projectionMat, modelViewMat,normalMat, modelMatnormals; // Uniform variables, set from the CPU-side main program
uniform mat4 viewMat;
out vec3 fNormal;
out vec3 fPosition;
out vec2 fTextCoord;
out vec3 fNormalGlobal;


void main() {
	

    vec4 posView=modelViewMat*vec4(vPosition,1.0);

    fPosition=posView.xyz;
    
    vec4 n = normalMat * vec4 (normalize (vNormal), 1.0);
    fNormal = normalize (n.xyz);
    fNormalGlobal=normalize(vec3(modelMatnormals*vec4(vNormal,1.0)));
    
    

    
    fTextCoord=vTexCoords;

    vec4 p = modelViewMat * vec4 (vPosition, 1.0);
    gl_Position =  projectionMat * p; // mandatory to fire rasterization properly
    
}