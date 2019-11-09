#ifdef SQ_VERTEX_SHADER

attribute vec4 inPosition;   
attribute vec2 inTexcoord;

uniform mat4 uMVPMatrix;
	  
varying vec2 texCoord;
  
void main(void)
{
	texCoord		= inTexcoord;
	gl_Position     = uMVPMatrix * inPosition;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

uniform sampler2D map;
varying vec2 texCoord;
    
void main (void)
{
	gl_FragColor = vec4(texture2D ( map, texCoord ).rgb, 1.0);
}
	
#endif
