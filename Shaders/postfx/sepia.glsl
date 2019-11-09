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

uniform sampler2D image;
const vec3 LUMINANCE_WEIGHTS = vec3(0.27, 0.67, 0.06);
const vec3 LightColor = vec3(1.0,0.9,0.5);
const vec3 DarkColor = vec3(0.2,0.05,0.0);

void main(void)
{
	vec3 col = texture2D ( image, texCoord ).xyz;

	float lum = dot(LUMINANCE_WEIGHTS,col);
	vec3 sepia = DarkColor*(1.0-lum) + LightColor*lum;
	gl_FragColor = vec4(sepia,1.0);
}

#endif
