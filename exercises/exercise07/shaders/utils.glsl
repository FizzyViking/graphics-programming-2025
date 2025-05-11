
// Returns camera position, extracted from view matrix
vec3 GetCameraPosition(mat4 viewMatrix)
{
	vec3 position = viewMatrix[3].xyz;
	position = -(transpose(viewMatrix) * vec4(position, 0)).xyz;
	return position;
}

// Gets vector halfway between two other vectors (typically, light and view)
vec3 GetHalfVector(vec3 v1, vec3 v2)
{
   return normalize(v1 + v2);
}

// Gets the direction from a point to another
vec3 GetDirection(vec3 fromPosition, vec3 toPosition)
{
	return normalize(toPosition - fromPosition);
}

// Returns the dot product, clamped only to positive values
float ClampedDot(vec3 v1, vec3 v2)
{
	return max(dot(v1, v2), 0);
}

// Constructs the 3D normal using only XY values (computing implicit Z)
vec3 GetImplicitNormal(vec2 normal)
{
	// (todo) 07.3: Obtain the implicit Z component of the normal
	vec3 normal3D = vec3(normal, 0);
	normal3D.z = sqrt(1 - dot(normal3D.xy, normal3D.xy));
	//normal3D = normalize(normal3D);
	return normal3D;
}

// Obtains a position in view space using the depth buffer and the inverse projection matrix
vec3 ReconstructViewPosition(sampler2D depthTexture, vec2 texCoord, mat4 invProjMatrix)
{
	// (todo) 07.4: Reconstruct the position, using the screen texture coordinates and the depth
	float depth = texture(depthTexture, texCoord).x;
	// normalize the depth value to -1..1 range
	depth = depth * 2.0 - 1.0;
	vec4 clipSpacePos = vec4(texCoord * 2.0 - 1.0, depth, 1.0);
	vec4 viewSpacePos = invProjMatrix * clipSpacePos;
	viewSpacePos /= viewSpacePos.w;
	return viewSpacePos.xyz;
}

