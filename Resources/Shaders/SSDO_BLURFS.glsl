#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
uniform bool withSSDO;
uniform sampler2D Li;
uniform sampler2D lightPass;

void main() 
{
    vec3 originalColor=texture(lightPass,TexCoords).rgb;

    vec2 texelSize = 1.0 / vec2(textureSize(Li, 0));
    vec3 result = vec3(0.0);
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(Li, TexCoords + offset).rgb;
        }
    }
    result/=4.0*4.0;
    if (withSSDO){
        result=vec3(0.0);
    }
    FragColor = vec4(originalColor+result,1.0);
}  

