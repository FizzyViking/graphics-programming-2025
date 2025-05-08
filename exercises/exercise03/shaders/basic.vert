#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

// (todo) 03.X: Add the out variables here
out vec3 Normal;
out vec3 Position;

// (todo) 03.X: Add the uniforms here
uniform mat4 WorldMatrix = mat4(1.0f);
uniform mat4 ViewProjMatrix;

void main()
{
	Position = (ViewProjMatrix * WorldMatrix * vec4(VertexPosition, 1.0)).xyz;
	//Position = (ViewProjMatrix * WorldMatrix * vec4(VertexNormal, 1.0)).xyz;
	Normal = (WorldMatrix * vec4(VertexNormal, 0.0f)).xyz;
	gl_Position = ViewProjMatrix * WorldMatrix * vec4(VertexPosition, 1.0);
}
