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

uniform sampler2D srcImage;
uniform sampler2D blurYImage;
uniform sampler2D adaptLumImage;
uniform float	exposure;
const float	blurBlendFactor   = 1.0;
const float	fGaussianScalar = 1.0;
const vec3 LUMINANCE_VECTOR  = vec3(0.2125, 0.7154, 0.0721);

void main(void)
{
	vec4 col = texture2D(srcImage, texCoord);
	vec4 bloom = texture2D(blurYImage, texCoord);
	vec4 lum = texture2D(adaptLumImage, vec2(0.5, 0.5));
	col += bloom * blurBlendFactor;

	//float key = (max(0.0, 1.5 - (1.5/(lum.x*0.9+1.0)))+0.1);

	// Reinhard's tone mapping equation (See Eqn#3 from 
	// "Photographic Tone Reproduction for Digital Images" for more details) is:
	//
	//      (      (   Lp    ))
	// Lp * (1.0f +(---------))
	//      (      ((Lm * Lm)))
	// -------------------------
	//         1.0f + Lp
	//
	// Lp is the luminance at the given point, this is computed using Eqn#2 from the above paper:
	//
	//        exposure
	//   Lp = -------- * HDRPixelIntensity
	//          l.r

	float Lp = ( exposure /** key*/ / lum.x ) * max( col.x, max( col.y, col.z ) );
	float LmSqr = (lum.y + fGaussianScalar * lum.y) * (lum.y + fGaussianScalar * lum.y);
	float toneScalar = ( Lp * ( 1.0 + ( Lp / ( LmSqr ) ) ) ) / ( 1.0 + Lp );

	col *= toneScalar;

	gl_FragColor = vec4(col.x, col.y, col.z, 1.0);
}

#endif