#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform vec2 ColorTextureScale;
uniform sampler2D ColorTextureWater;

void main()
{
	FragColor = Color * texture(ColorTextureWater, TexCoord * ColorTextureScale);
}
