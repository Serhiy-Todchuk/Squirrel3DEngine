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
const vec4	ShaftParams = vec4(0.1,2.0,0.1,2.0);

float	saturate(float val)
{
	return clamp(val,0.0,1.0);
}

vec4 blendSoftLight(vec4 a, vec4 b)
{
  vec4 c = 2.0 * a * b + a * a * (1.0 - 2.0 * b);
  vec4 d = sqrt(a) * (2.0 * b - 1.0) + 2.0 * a * (1.0 - b);
   
  // TODO: To look in Crysis what it the shit???
  //return ( b < 0.5 )? c : d;
  return any(lessThan(b, vec4(0.5,0.5,0.5,0.5)))? c : d;
}

void main(void)
{
	vec2	sunPosProj = sun_pos.xy;
	float	sign = sun_pos.w;

	vec2	sunVec = sunPosProj.xy - (texCoord - vec2(0.5,0.5));
	float	sunDist = saturate(sign) * saturate( 1.0 - saturate(length(sunVec) * ShaftParams.y ));

	sunVec *= ShaftParams.x * sign;

	vec4 accum;
	vec2 tc = texCoord;

	tc += sunVec;
	accum = texture2D(BlurSampler, tc);
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
	
 	accum.w += 1.0 - saturate(saturate(sign*0.1+0.9));

	vec4	cScreen = texture2D(FullSampler, texCoord);      
	vec4	cSunShafts = accum;

	float fShaftsMask = saturate(1.00001- cSunShafts.w) * ShaftParams.z * 2.0;
        
	float fBlend = cSunShafts.w;

	vec4 sunColor = vec4(0.9,0.8,0.6,1.0);

	accum =  cScreen + cSunShafts.xyzz * ShaftParams.w * sunColor * ( 1.0 - cScreen );
	accum = blendSoftLight(accum, sunColor * fShaftsMask * 0.5 + 0.5);

	gl_FragColor = accum;
}
#endif