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

uniform sampler2D colorBuffer;
uniform sampler2D blurMap;
uniform float	bloomBlendFactor;

void main(void)
{
	vec4 col = texture2D(colorBuffer, texCoord);
	vec4 bloom = texture2D(blurMap, texCoord);

	/*
	const vec3 LUMINANCE_WEIGHTS = vec3(0.27, 0.67, 0.06);
	float colorLum = dot(LUMINANCE_WEIGHTS, col);
	float bloomLum = dot(LUMINANCE_WEIGHTS, bloom);
	*/
	const vec4 one = vec4(1.0);
	const vec4 zero = vec4(0.0);

	vec4 bloomCol = clamp(bloom * bloomBlendFactor, zero, one);
	col = one - ((one - col) * (one - bloomCol));

	col.a = 1.0;

	gl_FragColor = col;
}

#endif
