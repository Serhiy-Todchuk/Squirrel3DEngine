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
uniform sampler2D ALumSampler;
uniform vec2	samples[16];
uniform	float	key;
const float  BRIGHT_PASS_THRESHOLD  = 2.5;  // Threshold for BrightPass filter
const float  BRIGHT_PASS_OFFSET     = 1.0; // Offset for BrightPass filter

void main(void)
{
	vec3 col = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < 16; i++)
	{
		col += texture2D(DownSampler, texCoord + samples[i]).xyz;
	}
	col *= 0.0625;
	
	float  ALum = texture2D(ALumSampler, vec2(0.5, 0.5) ).r;

	// Determine what the pixel's value will be after tone-mapping occurs
	col.xyz *= key/(ALum + 0.001);
	
	// Subtract out dark pixels
	col.xyz -= BRIGHT_PASS_THRESHOLD;
	
	// Clamp to 0
	col = max(col, 0.0);
	
	// Map the resulting value into the 0 to 1 range. Higher values for
	// BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
	// objects.
	col.xyz /= (BRIGHT_PASS_OFFSET + col.xyz);

	gl_FragColor = vec4(col, 1.0);

}

#endif