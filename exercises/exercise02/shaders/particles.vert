#version 330 core

layout (location = 0) in vec2 ParticlePosition;
// (todo) 02.X: Add more vertex attributes
layout (location = 1) in float ParticleSize;
layout (location = 2) in float Birth;
layout (location = 3) in float Duration;
layout (location = 4) in vec4 ParticleColor;
layout (location = 5) in vec2 Velocity;

// (todo) 02.5: Add Color output variable here
out vec4 VertParticleColor;

// (todo) 02.X: Add uniforms
uniform float CurrentTime;
uniform vec2 Gravity;

void main()
{
	float age = CurrentTime - Birth;
	gl_PointSize = age > Duration ? 0 : ParticleSize;
	VertParticleColor = ParticleColor;
	vec2 newPosition = ParticlePosition + Velocity * age + 0.5f * Gravity * age * age;
	gl_Position = vec4(newPosition, 0.0, 1.0);
}
