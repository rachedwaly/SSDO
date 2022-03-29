#version 450 core // Minimal GL version support expected from the GPU
#define M_PI 3.14159265358979323
in vec3 fNormal; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fPosition;


out vec4 colorResponse; // Shader output: the color response attached to this fragment




uniform mat4 normalMat,viewMat;

struct LightSource {
vec3 direction;
vec3 color;
float intensity;
vec3 position;
};

uniform LightSource lightSources[2];

uniform LightSource lightSource;

struct Material {
vec3 albedo;
float shininess;
float kd;
float ks;
float alpha; //roughness
};

uniform Material material;



float beckmannDistribution(vec3 n,vec3 wh, float alpha){
	float amplitude=1/(3.14*alpha*alpha*pow(dot(n,wh),4));
	float exponential=exp( (pow(max(dot(n,wh),0.0),2)-1)/ (pow(alpha,2)*pow(max(dot(n,wh),0.0),2) ) );
	float res=amplitude*exponential;
	return res;
}


float normalDistrubtion(vec3 n,vec3 wh,float alpha){
	float a=M_PI*pow(1+(pow(alpha,2)-1)*(pow(max(dot(n,wh),0.0),2)),2);
	float res=pow(alpha,2)/a;
	return res;
}



vec3 fresnelTerm(vec3 wi, vec3 wh){
	vec3 F0=vec3(0.02);
	float dot=pow(1-max(0.0,dot(wi,wh)),5);
	return F0+(1-F0)*dot;
}

float GGx(vec3 w,vec3 n,float alpha){

	float k=alpha*sqrt(2/M_PI);

	float dotProduct=max(dot(n,w),0.0);

	float result=dotProduct/(dotProduct*(1-k)+k);

	return result;

}





void main () {
	
	vec3 n=normalize(fNormal);
	
	vec3 wi=normalize(vec3(viewMat*vec4(normalize(-lightSource.direction),1.0)));
	
	
	vec3 wo = normalize (-fPosition);
	
	vec3 wh = normalize (wi + wo);
	
	
	vec3 fsi=normalDistrubtion(n,wh,material.alpha)*fresnelTerm(wi,wh)*GGx(wi,n,material.alpha)*GGx(wo,n,material.alpha);
	
	float dotProduct=max(dot(n,wh)*dot(n,wi),0.1); 

	

	vec3 fs=lightSource.color*material.albedo*fsi/(4*dotProduct)*max(0,dot(n,wi));

	
    vec3 fd=(material.kd/M_PI)*lightSource.color*material.albedo*max(0,dot(n,wi));

	vec3 f=fd+fs;

	colorResponse =10.0*vec4 (f, 1.0);
	
}