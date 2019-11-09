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
uniform vec2  samples[16];

vec4 SuppressLDR( vec4 c )
{
   if( c.r > 1.0 || c.g > 1.0 || c.b > 1.0 )
      return c;
   else
      return vec4( 0.0, 0.0, 0.0, 0.0 );
}


void main(void)
{
	vec3 col = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < 16; i++)
	{
		col += texture2D(DownSampler, texCoord + samples[i]).xyz;
	}
	col *= 0.0625;
	
	gl_FragColor = SuppressLDR( vec4(col, 1.0) );

}

#endif