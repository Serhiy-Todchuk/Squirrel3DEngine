#ifdef SQ_VERTEX_SHADER

varying	vec3 normal;

attribute vec4 inPosition;
uniform mat4 uMVPMatrix;

void main(void)
{
	normal			= inPosition.xyz;
	gl_Position     = uMVPMatrix * vec4 ( inPosition.xyz, 1.0 );
}

#endif
#ifdef SQ_FRAGMENT_SHADER

varying	vec3 normal;

uniform samplerCube skyMap;

void main (void)
{
	gl_FragColor = textureCube ( skyMap, normal ).rgba;
}
  
#endif