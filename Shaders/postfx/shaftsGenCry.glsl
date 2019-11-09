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
uniform sampler2DShadow DepthSampler;

float	saturate(float val)
{
	return clamp(val,0.0,1.0);
}

void main(void)
{
	float sceneDepth	= shadow2D(DepthSampler, texCoordz).r;
	vec4  scene		= texture2D(FullSampler, texCoord);
	float fShaftsMask 	= (1.0 - sceneDepth);

	gl_FragColor = vec4( scene.xyz * saturate(sceneDepth), fShaftsMask );
}
#endif