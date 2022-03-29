#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gNormalGlobal;

in vec3 fNormal;
in vec3 fPosition;
in vec2 fTextCoord;
in vec3 fNormalGlobal;

struct Material {
vec3 albedo;
float roughness;
};
uniform Material material;
uniform bool hasTexture;
uniform int s;

uniform sampler2D textureAlbedo;
uniform sampler2D textureRoughness;
uniform sampler2D textureAlbedo1;
uniform sampler2D textureRoughness1;
uniform sampler2D textureAlbedo2;
uniform sampler2D textureRoughness2;

void main()
{
    gPosition=fPosition;
    gNormal=normalize(fNormal);
    gNormalGlobal=normalize(fNormalGlobal);
    vec3 color;
    float alpha;
    if (hasTexture){
    if (s==0){
    color=texture(textureAlbedo,fTextCoord).rgb;
    alpha=texture(textureRoughness,fTextCoord).r;
    }else if(s==1){
    color=texture(textureAlbedo1,fTextCoord).rgb;
    alpha=texture(textureRoughness1,fTextCoord).r; 
    }else{
    color=texture(textureAlbedo2,fTextCoord).rgb;
    alpha=texture(textureRoughness2,fTextCoord).r;  
    }
    }
    else{
    color=material.albedo;
    alpha=material.roughness;
    }
    gAlbedo.rgb=color;
    gAlbedo.a=alpha;
}