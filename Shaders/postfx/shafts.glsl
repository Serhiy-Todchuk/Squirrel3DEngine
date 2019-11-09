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
uniform sampler2D BlurSampler;
uniform vec4 sun_pos;
const vec4	ShaftParams	= vec4(0.05,1.0,0.05,1.0);
const vec4	sunColor	= vec4(0.9,0.8,0.6,1.0);

float	saturate(float val)
{
	return clamp(val,0.0,1.0);
}
void main(void)
{
	vec2	sunPosProj = sun_pos.xy;
	float	sign = sun_pos.w;

	vec2 tc = texCoord;

	vec2	sunVec = sunPosProj.xy - tc + vec2(0.5,0.5);
	float	sunDist = saturate(sign) * ( 1.0 - saturate(dot(sunVec,sunVec) * ShaftParams.y ));

	sunVec *= ShaftParams.x * sign;

	tc += sunVec;
	vec4 accum = texture2D(BlurSampler, tc);
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.875;
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.75;
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.625;
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.5;
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.375;
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.25;
	tc += sunVec;
	accum += texture2D(BlurSampler, tc) * 0.125;

	accum  *= 0.25 * vec4(sunDist,sunDist,sunDist,1.0);

	vec4	cScreen = texture2D(FullSampler, texCoord);
	accum = cScreen + accum * ShaftParams.w * sunColor * ( 1.0 - cScreen );

	gl_FragColor = accum;
}

#endif