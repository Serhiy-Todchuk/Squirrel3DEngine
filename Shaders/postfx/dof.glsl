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
uniform sampler2D Blur;
uniform sampler2DShadow DepthSampler;

uniform float range;
uniform float focus;

uniform float uCamFar;
uniform float uCamNear;

void main(void)
{
	float depth = shadow2D(DepthSampler, vec3(texCoord, 0.0)).r;

	float z_c1 = uCamFar*uCamNear/(uCamFar-uCamNear); 
	float z_c2=(uCamFar+uCamNear)/(2.0*(uCamFar-uCamNear)); 
	depth = z_c1/(-z_c2-0.5+depth);

	vec4 sharp = texture2D(FullSampler, texCoord);
	vec4 blur  = texture2D(Blur, texCoord);

	vec4 final = mix(sharp, blur, clamp(range * abs(focus + depth), 0.0, 1.0));
	
	final.a = 1.0;
	
	gl_FragColor = final;
}

#endif