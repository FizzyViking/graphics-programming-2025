#version 330 core

out vec4 FragColor;

// (todo) 02.5: Add Color input variable here
in vec4 VertParticleColor;

void main()
{
	// (todo) 02.3: Compute alpha using the built-in variable gl_PointCoord
	
	float alpha = 1.0f - length((gl_PointCoord * 2.0f - 1.0f));

	FragColor = vec4(VertParticleColor.x, VertParticleColor.y, VertParticleColor.z, VertParticleColor.w * alpha);
}
