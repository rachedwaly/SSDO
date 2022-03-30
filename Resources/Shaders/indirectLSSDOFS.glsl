#version 450 core
#define M_PI 3.14159265358979323
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D lightPass;

uniform vec3 samples[200];
uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform vec2 noiseScale; 
uniform mat4 projection;

vec3 calculateIndirectLight(float cosThetaSi,float cosThetaRi,float di,vec3 color){
    return (cosThetaSi*cosThetaRi)*color*pow((di/radius),2);

} 

void main()
{
    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, TexCoords).xyz; //in view space
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb); // in view space
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz); //choose a random orientation vector
    vec3 originalColor=texture(lightPass, TexCoords).rgb;

    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);

    mat3 TBN = mat3(tangent, bitangent, normal);
    // looping over all the samples and calculating the occlusion occurences
    float occlusion = 0.0;
    vec3 Ldi=vec3(0.0);
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) 
        vec4 offset = vec4(samplePos, 1.0);
        
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        
        offset.xyz = offset.xyz*0.5+0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        vec3 sampleNormal=normalize(texture(gNormal,offset.xy).xyz); //get sample's normal in view space
        
        vec3 sampleColor=texture(lightPass,offset.xy).xyz; //get sample's color in the screen
        
        float di=length(fragPos-samplePos);

        vec3 transmittance=normalize(fragPos-samplePos); //in view space
        
        //remove back facing patches
        float cosThetaSi=max(dot(transmittance,sampleNormal),0.0);
        float cosThetaRi=max(dot(-transmittance,normal),0.0);
        

       

        float rangeCheck=radius / abs(fragPos.z - sampleDepth);

        //Verify occlusion and range
        if ((sampleDepth>=samplePos.z+bias)&&(rangeCheck>=1.0)) { 
            occlusion++;
            Ldi+=calculateIndirectLight(cosThetaSi,cosThetaRi,di,sampleColor);
        }
            
    }
    FragColor = vec4(Ldi,1.0); 
}