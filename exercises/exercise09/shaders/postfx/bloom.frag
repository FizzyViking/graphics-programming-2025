//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D SourceTexture;
uniform float Intensity;
uniform vec2 Range;

void main()
{
	vec3 color = texture(SourceTexture, TexCoord).rbg;
	float luminance = GetLuminance(color);
	luminance = luminance <= Range.x ? 0 : luminance;
	luminance = luminance >= Range.x ? 1 : luminance;

	FragColor = vec4(color * Intensity * luminance,1);
}
