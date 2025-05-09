#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in float Height;

out vec4 FragColor;

uniform vec4 Color;
uniform vec2 ColorTextureScale;
uniform sampler2D ColorTextureGrass;
uniform sampler2D ColorTextureDirt;
uniform sampler2D ColorTextureRock;
uniform sampler2D ColorTextureSnow;
uniform vec2 HeightRange1;
uniform vec2 HeightRange2;
uniform vec2 HeightRange3;

float interpolateHeight(vec2 range, float height) {
	if (height <= range.x) return 0.0f;
	if (height >= range.y) return 1.0f;
	return clamp((height - range.x) / (range.y - range.x),0, 1);
}

void main()
{
	vec4 FinalColor = texture(ColorTextureDirt, (TexCoord * ColorTextureScale));
	vec4 Color1 = texture(ColorTextureGrass, (TexCoord * ColorTextureScale));
	vec4 Color2 = texture(ColorTextureRock, (TexCoord * ColorTextureScale));
	vec4 Color3 = texture(ColorTextureSnow, (TexCoord * ColorTextureScale));

	FinalColor = mix(FinalColor, Color1, interpolateHeight(HeightRange1, Height));
	FinalColor = mix(FinalColor, Color2, interpolateHeight(HeightRange2, Height));
	FinalColor = mix(FinalColor, Color3, interpolateHeight(HeightRange3, Height));

	FragColor = Color * FinalColor;
}
