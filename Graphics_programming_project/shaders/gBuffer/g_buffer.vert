#version 330 core
layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 aNormal;

uniform bool InvertedNormals;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjMatrix;
uniform mat3 NormalMatrix;

void main()
{
    vec4 viewPos = ViewMatrix * ModelMatrix * vec4(VertexPosition, 1.0);
    FragPos = viewPos.xyz; 
    TexCoords = VertexTexCoord;
    
    aNormal = NormalMatrix * (InvertedNormals ? -VertexNormal : VertexNormal);
    
    gl_Position = ProjMatrix * viewPos;
}