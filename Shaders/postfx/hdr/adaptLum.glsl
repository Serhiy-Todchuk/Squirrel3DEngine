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

uniform sampler2D CurLum;
uniform sampler2D AdaptLum;
uniform float uDeltaTime;

void main(void)
{
	vec3	Clum = texture2D(CurLum, vec2(0.5, 0.5)).xyz;
	vec3	Alum = texture2D(AdaptLum, vec2(0.5, 0.5)).xyz;

/*	float	sigma = 0.04/(0.04 + Clum.x);
	float	tau = sigma*0.4 + (1.0 - sigma)*0.1;
	vec3	col = Alum + (Clum - Alum) * (1.0 - exp(-uDeltaTime/tau));
*/

	const vec3 quarter = vec3(0.25);
	vec3	rootAlum = pow( Alum, quarter );
	vec3	rootClum = pow( Clum, quarter );
	vec3	col = pow( rootAlum + ( rootClum - rootAlum ) * ( 1.0 - pow( 0.98, 30 * uDeltaTime ) ), vec3(4.0));
	//vec3	col = Alum + (Clum - Alum) * (1.0 - pow(0.98, 30.0 * uDeltaTime));
	col.x = clamp(col.x, 0.25, 5.0);
	col.y = clamp(col.y, 0.5, 2.0);
	gl_FragColor = vec4(col.x,col.y, 0.0, 1.0);
}

#endif
