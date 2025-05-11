//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform sampler2D DepthTexture;
uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D OthersTexture;
uniform mat4 InvViewMatrix;
uniform mat4 InvProjMatrix;

void main()
{
	vec3 albedo = texture(AlbedoTexture, TexCoord).rgb;
	vec3 position = ReconstructViewPosition(DepthTexture, TexCoord, InvProjMatrix);
	vec3 normal = GetImplicitNormal(texture(NormalTexture, TexCoord).xy);
	vec4 others = texture(OthersTexture, TexCoord);

	SurfaceData data;
	data.normal = (InvViewMatrix * vec4(normal, 0)).xyz;
	data.reflectionColor = albedo;
	data.ambientReflectance = others.x;
	data.diffuseReflectance = others.y;
	data.specularReflectance = others.z;
	data.specularExponent = (1.0f / others.w) - 1.0f;

	vec3 viewDir = normalize(position - vec3(0));
	position = (InvViewMatrix * vec4(position,1)).xyz;
	viewDir = (InvViewMatrix * vec4(viewDir,0)).xyz;

	FragColor = vec4(ComputeLighting(position, data, viewDir, true), 1.0f);
}
