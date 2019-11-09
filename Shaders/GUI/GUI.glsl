#ifdef SQ_VERTEX_SHADER

#ifdef CLIPPING
//up, down, left, right
varying	vec4 clipping;
varying	vec2 clipPos;
#endif

attribute vec4 inPosition;
uniform mat4 uMVPMatrix;

#ifdef VERTEX_COLOR
attribute vec4 inColor;
varying	vec4 vColor;
#endif

#ifdef TEXTURE
attribute vec2 inTexcoord;
varying	vec2 texCoord;
#endif

#ifdef CLIPPING
attribute vec4 inTangentBinormal;
#endif

void main(void)
{
	
#ifdef TEXTURE
	texCoord		= inTexcoord;
#endif
	
#ifdef CLIPPING
	clipping		= inTangentBinormal;
	clipPos			= inPosition.xy;
#endif
	
#ifdef VERTEX_COLOR
	vColor			= inColor;
#endif
	gl_Position     = uMVPMatrix * inPosition;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

#ifdef CLIPPING
//up, down, left, right
varying	vec4 clipping;
varying	vec2 clipPos;
#endif

#ifdef VERTEX_COLOR
varying	vec4 vColor;
#endif

#ifdef TEXTURE
varying	vec2 texCoord;
uniform sampler2D texture;
#endif

uniform vec4 uColor;

void main (void)
{
	vec4 c = uColor;
	
#ifdef VERTEX_COLOR
	c = c * vColor;
#endif
	
#ifdef TEXTURE
	vec4 texFetch = texture2D ( texture, texCoord );
	
	c.a = c.a * texFetch.a;
#endif
	
#ifdef CLIPPING
	if(clipPos.x < clipping.z || clipPos.x > clipping.w || clipPos.y < clipping.x || clipPos.y > clipping.y)
		c.a = 0.0;
#endif

	if(c.a == 0.0)
		discard;

	gl_FragColor = c;
}

#endif