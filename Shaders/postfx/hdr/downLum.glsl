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

uniform sampler2D DownSampler;
uniform vec2 samples[16];

void main(void)
{
	float	lum = 0.0;
	vec4	color;
	float	maximum = 0.0;
	for(int i = 0; i < 16; i++)
	{
		color = texture2D(DownSampler, texCoord + samples[i]);                       
		maximum = max( maximum, color.g );
		lum += color.r;
	}
	lum *= 0.0625;
	
	gl_FragColor = vec4(lum, maximum, 0.0, 1.0);
}

#endif