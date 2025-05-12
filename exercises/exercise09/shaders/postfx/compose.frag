//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D SourceTexture;
uniform sampler2D BloomTexture;
uniform float Exposure;
uniform float Contrast;
uniform float HueShift;
uniform float Saturation;
uniform vec3 ColorFilter;

void main()
{
	vec3 hdrColor = texture(SourceTexture, TexCoord).rgb;
	hdrColor += texture(BloomTexture, TexCoord).rgb;
	vec3 color = vec3(1.0f) - exp(-hdrColor * Exposure);
	
	// Contrast
	color = clamp((color - vec3(0.5)) * Contrast + vec3(0.5), 0, 1);

	// Hue Shift
	color = RGBToHSV(color);
	color.x = fract(color.x + HueShift + 1.0f);
	color = HSVToRGB(color);

	// Saturation
	float luminance = GetLuminance(color);
	color = clamp(((color - vec3(luminance)) * Saturation) + luminance,0,1);

	FragColor = vec4(color * ColorFilter, 1);
}
