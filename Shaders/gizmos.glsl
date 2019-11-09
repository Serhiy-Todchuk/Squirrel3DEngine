#ifdef SQ_VERTEX_SHADER

#ifdef LIGHTING
varying	vec3 normal;
#endif

#ifdef TEXTURE
varying	vec2 texCoord;
#endif

attribute vec4 inPosition;
attribute vec2 inTexcoord;
attribute vec3 inNormal;

uniform mat3 uNormalMatrix;
uniform mat4 uMVPMatrix;

void main(void)
{
#ifdef LIGHTING
	normal = normalize( (uNormalMatrix * inNormal).xyz );
#endif

#ifdef TEXTURE
	texCoord = inTexcoord;
#endif

	gl_Position     = uMVPMatrix * inPosition;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

uniform	vec3 light;
uniform	float ambient;

#ifdef LIGHTING
varying	vec3 normal;
#endif

#ifdef TEXTURE
varying	vec2 texCoord;
uniform sampler2D texture;
#endif

uniform vec4 uColor;

void main (void)
{
	vec4 c = uColor;
	
#ifdef TEXTURE
	c = c * texture2D ( texture, texCoord );
#endif

#ifdef LIGHTING
	float intensity = max( dot( normalize( normal ), light ), 0.0 );
	intensity = (1.0 - ambient) * intensity + ambient;
	c.xyz = c.xyz * intensity;
#endif
	
	gl_FragColor = c;
}

#endif