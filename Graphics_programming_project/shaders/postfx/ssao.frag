#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

uniform int KernelSize = 64;
uniform float Radius = 0.5f;
uniform float Bias = 0.025f;

const vec2 noiseScale = vec2(1024/4.0, 1024/4.0); 

uniform mat4 ProjMatrix;

void main()
{
    // Extract information from textures
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // tangent-space to view-space matrix
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    // Calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < KernelSize; ++i)
    {
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * Radius; 
        
        vec4 offset = vec4(samplePos, 1.0);
        offset = ProjMatrix * offset; 
        offset.xyz /= offset.w; 
        offset.xyz = offset.xyz * 0.5 + 0.5; 
        
        // Depth of kernel sample
        float depth = texture(gPosition, offset.xy).z; 
        
        float rangeCheck = smoothstep(0.0, 1.0, Radius / abs(fragPos.z - depth));
        occlusion += (depth >= samplePos.z + Bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / KernelSize);
    
    FragColor = occlusion;
}