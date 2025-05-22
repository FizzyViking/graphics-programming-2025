#version 330 core
layout (location = 0) out vec3 VertexPosition;
layout (location = 1) out vec3 VertexNormal;
layout (location = 2) out vec4 VertexAlbedo;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D SourceTexture;
uniform vec3 Color;

void main()
{    
    VertexPosition = FragPos;
    VertexNormal = normalize(Normal);
    VertexAlbedo = vec4(Color.rgb * texture(SourceTexture, TexCoord).rgb, 1);
}