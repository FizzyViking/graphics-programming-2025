#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D ColorTexture;
uniform vec3 AmbientColor;
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;
uniform vec3 LightDirection;
uniform float AmbientReflection;
uniform float DiffuseReflection;
uniform float SpecularReflection;
uniform float SpecularExponent;
uniform float LightConstant;
uniform float LightLinear;
uniform float LightQuadratic;

vec3 GetAmbientReflection(vec3 objectColor) {
	return AmbientColor * AmbientReflection * objectColor;
}

vec3 GetDiffuseReflection(vec3 lightVector, vec3 worldNormal, vec3 objectColor) {
	float diff = max(dot(worldNormal, lightVector), 0.0f);
	return LightColor * DiffuseReflection * objectColor * diff;
}

vec3 GetSpecularReflection(vec3 viewDir, vec3 reflectDir) {
	return LightColor * SpecularReflection * pow(max(dot(viewDir, reflectDir), 0.0), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec3 objectColor, vec3 lightVector, vec3 worldNormal, vec3 viewDir, vec3 reflectDir) {
	float distance = length(LightPosition - WorldPosition);
	float attenuation = 1.0 / (LightConstant + LightLinear * distance + LightQuadratic * (distance * distance));
	return (GetAmbientReflection(objectColor) * attenuation + 
	GetDiffuseReflection(lightVector, worldNormal, objectColor) * attenuation +
	GetSpecularReflection(viewDir, reflectDir) * attenuation
	);
}

void main()
{
	vec3 viewDir = normalize(CameraPosition - WorldPosition);
	vec3 lightVector = normalize(LightPosition - WorldPosition);
	//vec3 lightVector = normalize(-LightDirection);
	vec3 reflectDir = reflect(-lightVector, normalize(WorldNormal));
	FragColor = vec4(GetBlinnPhongReflection((texture(ColorTexture, TexCoord) * Color).rgb, lightVector, normalize(WorldNormal), viewDir, reflectDir), 1.0f);
}
