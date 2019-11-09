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

const vec3 LUMINANCE_VECTOR  = vec3(0.2125, 0.7154, 0.0721);
uniform sampler2D DownSampler;
uniform	vec2 samples[9];

void main(void)
{
	float	lum = 0.0;
	float	GreyValue = 0.0;
	float	maximum = 0.0;
	vec3	color;

	for(int i = 0; i < 9; i++)
	{
		color = texture2D(DownSampler, texCoord + samples[i]).xyz;
		lum += log(dot(color, LUMINANCE_VECTOR) + 0.0001);
		GreyValue = max( color.r, max( color.g, color.b ) );
		maximum = max( maximum, GreyValue );
	}
	lum *= 0.111111;

	gl_FragColor = vec4(lum, maximum, 0.0, 1.0);
}

#endif