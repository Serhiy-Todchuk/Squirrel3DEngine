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
uniform vec2 samples[16];

vec3 SuppressLDR( vec3 c )
{
	float luminance = max(c.r, max(c.g, c.b));
	float scale = pow(luminance, 32.0);

	if ( luminance > 1.0 )
		return c;
	else
		return c * scale;
}

void main(void)
{
	vec3 col = vec3(0.0);
	for (int i = 0; i < 16; i++)
	{
		col += texture2D(colorBuffer, texCoord + samples[i]).rgb;
	}
	col *= 0.0625;

	gl_FragColor = vec4(SuppressLDR( col * 1.01 ), 1.0);
}

#endif
