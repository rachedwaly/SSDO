#version 450 core // Minimal GL version support expected from the GPU
#define M_PI 3.14159265358979323

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoRoughness;
uniform sampler2D gNormalGlobal;

struct LightSource {
vec3 direction;
vec3 color;
float intensity;
};

uniform LightSource lights[50];
uniform int numOfLights;

uniform vec3 viewPos;








uniform LightSource lightSource;




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


void main()
{
    float AmbientOcclusion = texture(ssao, TexCoords).r;


	vec3 albedo = texture(gAlbedoRoughness, TexCoords).rgb;
	
	
	vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 n = texture(gNormalGlobal, TexCoords).rgb;

	vec3 ntest = texture(gNormal, TexCoords).rgb;
	
	
    float alpha=texture(gAlbedoRoughness, TexCoords).a; //roughness

	vec3 res=vec3(0,0,0);
	
	for (int i=0; i < numOfLights; i++){
		LightSource light = lights[i];
		vec3 wi=normalize(-light.direction);
		vec3 wo=normalize(-FragPos);
		vec3 wh = normalize (wi + wo);
		vec3 fsi=normalDistrubtion(n,wh,alpha)*fresnelTerm(wi,wh)*GGx(wi,n,alpha)*GGx(wo,n,alpha);
		float dotProduct=max(dot(n,wh)*dot(n,wi),0.1); 
		vec3 fs=light.color*albedo*fsi/(4*dotProduct)*max(0,dot(n,wi));
		vec3 fd=(0.4/M_PI)*light.color*albedo*max(0,dot(n,wi));	
		vec3 f=fd+fs;
		res+=light.intensity*f;
	}
	//FragColor=vec4(res,1.0);
	FragColor=vec4(1.0);


}