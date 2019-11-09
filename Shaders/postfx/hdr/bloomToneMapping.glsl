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

varying vec2 texCoord;

uniform sampler2D FullSampler;
uniform sampler2D yBlur;

void main(void)
{
	vec4 col = texture2D(FullSampler, texCoord);
	vec4 bloom = texture2D(yBlur, texCoord);

	col += bloom * 0.8;

	gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
}

#endif