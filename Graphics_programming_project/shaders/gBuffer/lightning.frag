#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// Uniforms
uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Albedo;
uniform sampler2D ssao;

uniform float AmbientReflection;
uniform vec3 AmbientColor;
uniform float DiffuseReflection;
uniform vec3 LightColor;
uniform vec3 LightPosition;
float SpecularReflection = 1;
float SpecularExponent = 100;
float Linear = 0.035;
float Quadratic = 0.044;

uniform int SSAOActive;

void main()
{             
    // Extract data from gbuffer
    vec3 FragPos = texture(Position, TexCoords).rgb;
    vec3 Normal = texture(Normal, TexCoords).rgb;
    vec3 objectColor = texture(Albedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;

    // Calculate Lighting (Blinn-Phong)
    vec3 ambient = AmbientColor * AmbientReflection;
    if (SSAOActive == 1) {
        ambient *= AmbientOcclusion; // Here we add the occlusion factor
    } 
    
    vec3 lighting = ambient; 
    vec3 viewVector  = normalize(-FragPos);

    // Diffuse
    vec3 lightVector = normalize(LightPosition - FragPos);
    vec3 diffuse = max(dot(Normal, lightVector), 0.0) * DiffuseReflection * LightColor;

    // Specular
    vec3 halfVector = normalize(lightVector + viewVector); 
    float spec = pow(max(dot(Normal, halfVector), 0.0), SpecularExponent);
    vec3 specular = LightColor * spec * SpecularReflection;

    float distance = length(LightPosition - FragPos);
    float attenuation = 1.0 / (1.0 + Linear * distance + Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    FragColor = vec4(lighting, 1.0f);
}