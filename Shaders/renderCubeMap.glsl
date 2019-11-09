#ifdef SQ_VERTEX_SHADER

attribute vec4 inPosition;   

uniform mat4 uMVPMatrix;
	  
varying vec3 normal;
  
void main(void)
{
	normal			= normalize( inPosition.xyz );
	gl_Position     = uMVPMatrix * inPosition;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

uniform samplerCube map;
varying vec3 normal;
    
void main (void)
{
	gl_FragColor = vec4(textureCube ( map, normal ).rgb, 1.0);
}
	
#endif