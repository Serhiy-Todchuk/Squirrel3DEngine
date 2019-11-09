#ifdef SQ_VERTEX_SHADER

varying	vec3 pos;
varying	vec2 texCoord;
varying	vec4 color;

#ifdef SOFT
varying	vec4 centerAndSize;
#endif

uniform mat4 uModelViewMatrix;
uniform mat4 uMVPMatrix;

uniform vec3 cameraPos;
uniform vec3 cameraUp;

attribute vec4 inPosition;
attribute vec4 inColor;
attribute vec2 inTexcoord;
attribute vec2 inTexcoord2;

void main(void)
{
	float	size	= inTexcoord2.x;
	
	vec3	center	= (uModelViewMatrix * inPosition).xyz;
	
	vec3	eye		= normalize(cameraPos - center);
	vec3	right	= normalize(cross(eye, cameraUp));
	vec3	up		= normalize(cross(right, eye));
	
	mat3	orientBasis;
	
	orientBasis[0] = right;
	orientBasis[1] = up;
	orientBasis[2] = eye;
	
	vec2 r	= vec2(1, 0);
	vec2 u	= vec2(0, 1);	
	
#ifdef ROTATION
	float	angle = inTexcoord2.y;

	float	c = cos(angle);
	float	s = sin(angle);
	
	mat2	rotation = mat2(c, s, -s, c);

	r	= rotation * r;
	u	= rotation * u;
#endif

	right	= (orientBasis * vec3(r.x, r.y, 0.0))	* (size * (inTexcoord.x - 0.5));
	up		= (orientBasis * vec3(u.x, u.y, 0.0))	* (size * (inTexcoord.y - 0.5));

	pos             = inPosition.xyz + right + up;
	color			= inColor;
	gl_Position     = uMVPMatrix * vec4 ( pos, 1.0 );
	texCoord		= inTexcoord;
	
#ifdef SOFT
	centerAndSize	= vec4 ( center, size );
#endif
}

#endif
#ifdef SQ_FRAGMENT_SHADER

varying	vec3 pos;
varying	vec2 texCoord;
varying	vec4 color;

#ifdef SOFT
varying	vec4 centerAndSize;
#endif

#ifdef SOFT
uniform sampler2DRect	depthMap;
#endif

uniform sampler2D		particleMap;

void main (void)
{
	float	dz = 1.0;

#ifdef SOFT
	const float tau = 0.7;
	const float n = 0.1;
	const float f = 100.0;
	float scale = 0.3;
	
	float	d = distance ( pos, centerAndSize.xyz );
	float	r = centerAndSize.w * 0.5;
	
	if ( d >= r )
		discard;
		
	float	w   = r*r - d*d;
	float	zs  = texture2DRect ( depthMap, gl_FragCoord.xy ).r;
	
	float d1 = n*f/(f - zs*(f-n));
	float d2 = n*f/(f - gl_FragCoord.z*(f-n));

	dz  = min ( 0.5, scale * ( d1 - d2 ) / r );
#endif
	
	vec4	clr = color * texture2D ( particleMap, texCoord );
	
	gl_FragColor = vec4 ( clr.rgb, dz*clr.a );
}

#endif