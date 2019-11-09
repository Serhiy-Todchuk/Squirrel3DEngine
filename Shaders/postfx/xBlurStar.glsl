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

uniform sampler2D BlurXSampler;
uniform float	blurness;

void main(void)
{
    vec2 tx  = texCoord;
    vec2 dx  = vec2 (0.01953,0.01953)*blurness;
    vec2 sdx = dx;
    vec4 sum = texture2D ( BlurXSampler, tx ) * 0.134598;

    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.127325;
    sdx += dx;
    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.107778;
    sdx += dx;
    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.081638;
    sdx += dx;
    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.055335;
    sdx += dx;
    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.033562;
    sdx += dx;
    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.018216;
    sdx += dx;
    sum += (texture2D ( BlurXSampler, tx + sdx ) + texture2D ( BlurXSampler, tx - sdx ) )* 0.008847;
    sdx += dx;
	
	sum.a = 1.0;

    gl_FragColor = sum;

/*
	vec2 BlurOffset = vec2 (0.01953,0.01953);
	float PixelWeight[8] = {0.134598,0.127325,0.107778,0.081638,0.055335,0.033562,0.018216,0.008847};
	vec4 color = texture2D( BlurXSampler, texCoord );

	for( int i = 0; i < 8; ++i )
	{
		color += texture2D( BlurXSampler, texCoord + ( BlurOffset * i ) ) * PixelWeight[i];
		color += texture2D( BlurXSampler, texCoord - ( BlurOffset * i ) ) * PixelWeight[i];
	}
	gl_FragColor = color;*/

}

#endif