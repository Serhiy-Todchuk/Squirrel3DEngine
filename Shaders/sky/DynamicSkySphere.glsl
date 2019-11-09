#ifdef SQ_VERTEX_SHADER

varying	vec3 normal;

attribute vec4 inPosition;
uniform mat4 uMVPMatrix;

void main(void)
{
	normal			= inPosition.xyz;
	gl_Position     = uMVPMatrix * vec4 ( inPosition.xyz, 1.0 );
}

#endif
#ifdef SQ_FRAGMENT_SHADER

varying	vec3 normal;

uniform vec3 skyTopColor;
uniform vec3 skyBottomColor;

uniform vec3 orbColor;

uniform vec3 orbDir;
uniform float orbRadius;

const float orbThreshold = 0.002f;

void main (void)
{
	vec3 n = normalize(normal);
	
	float orbDotProduct = 2.0 - (1.0 + dot(n, orbDir));	//0..2, 0 - orb center, 1 - half distant from orb, 2 - farest point from orb
	
	float orbInvHeight = - (orbDir.y - 1.0);	//0..2, 0 - zenit, 1 - horizont, 2 - bottom

	float skyInvHeight = - (n.y - 1.0);		//0..2, 0 - zenit, 1 - horizont, 2 - bottom
	
	skyInvHeight = mix(skyInvHeight, orbDotProduct, orbInvHeight * 0.5);
	
	float skyInvShininess = skyInvHeight * 0.5 + orbInvHeight * 0.5;
	vec3 skyBgrColor = mix(skyTopColor, skyBottomColor, skyInvShininess);
	
	float r = min(2.0 - orbInvHeight, 1.0) * 0.25;
	
	float orbAtt = 1.0 - clamp(orbDotProduct / r, 0.0, 1.0);
	
	vec3 skyColor = mix(skyBgrColor, orbColor * 1.0, pow(orbAtt, 2.0));

	float orbColorWeight = 1.0 - clamp((orbRadius - orbDotProduct) / orbThreshold, 0.0, 1.0);

	skyColor = mix(orbColor * 1.4, skyColor, orbColorWeight * orbColorWeight);

	//if(orbDotProduct < orbRadius) skyColor = orbColor * 1.2;

	float shine = 1.0 - skyInvShininess * 0.5;
	float alpha = 1.0;
	if(shine < 0.4) alpha = shine * 2.5;
	
	gl_FragColor = vec4( skyColor, alpha );
}
  
#endif