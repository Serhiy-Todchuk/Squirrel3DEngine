//light orientation and size properties (need here to calculate lightVec vector)

#if defined(POINT_LIGHT) || defined(SPOT_LIGHT)
uniform vec3 lightPos;
uniform float lightRad;
#endif

#if defined(DIR_LIGHT) || defined(SPOT_LIGHT)
uniform vec3 lightDir;
#endif

#if defined(SPOT_LIGHT)
uniform float lightInnerCone;
uniform float lightOuterCone;
#endif

void computeLightVecAndIntensity(vec3 lGlobalPos, out vec3 outLightVec, out float outIntensity)
{
#if defined(POINT_LIGHT) || defined(SPOT_LIGHT)
	outLightVec			= lightPos - lGlobalPos;
	float distToLight	= length( outLightVec );
	outLightVec			= outLightVec / distToLight;
	outIntensity		= max ( 1.0 - ( (distToLight*distToLight) / (lightRad*lightRad) ),0.0);
#elif defined(DIR_LIGHT)
	outLightVec			= lightDir;
	outIntensity		= clamp(dot(lightDir, vec3(0, 1, 0)) + 1.0, 0.0, 1.0);
#endif
	
#if defined(SPOT_LIGHT)
	float cone			= dot(-outLightVec, lightDir);
	float coneIntensity	= (cone - lightOuterCone) / (lightInnerCone - lightOuterCone);
	outIntensity *= clamp(coneIntensity, 0.0, 1.0);
#endif
}

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec4 specular;
uniform float shininess;

#if defined(LIT_RAMP)

uniform sampler2D rampMap;

vec3 performLighting(vec3 normal, vec3 eyeVec, vec3 lightVec, float lightIntensity, vec3 diffColor, vec3 specColor)
{
	float NdotL		= dot ( normal, lightVec );
	
	float rampCoord	= NdotL * 0.5 + 0.5;
	
	vec3 diff		= diffuse * texture2D(rampMap, vec2(rampCoord)).rgb * lightIntensity;
	
	return (diffColor * (ambient + diff));
}

#elif defined(LIT_LAMBERT)

vec3 performLighting(vec3 normal, vec3 eyeVec, vec3 lightVec, float lightIntensity, vec3 diffColor, vec3 specColor)
{
	float NdotL		= dot ( normal, lightVec );

	float clampedNdotL		= max ( NdotL, 0.0 );

	vec3 diff		= diffuse * clampedNdotL * lightIntensity;

	return ((diffColor * (ambient + diff)));
}

#elif defined(LIT_PHONG)

vec3 performLighting(vec3 normal, vec3 eyeVec, vec3 lightVec, float lightIntensity, vec3 diffColor, vec3 specColor)
{
	vec3 reflVec    = reflect ( -eyeVec, normal );
	float NdotL		= dot ( normal, lightVec );
	float LdotR		= dot ( lightVec, reflVec );

	float clampedNdotL		= max ( NdotL, 0.0 );

	vec3 diff		= diffuse * clampedNdotL * lightIntensity;
	vec3 spec		= specular.xyz * specColor * pow ( max ( LdotR, 0.0 ), shininess ) * lightIntensity * clampedNdotL;

	return ((diffColor * (ambient + diff)) + spec);
}

#else //LIT_BLINN

vec3 performLighting(vec3 normal, vec3 eyeVec, vec3 lightVec, float lightIntensity, vec3 diffColor, vec3 specColor)
{
	vec3 halfVec	= normalize(lightVec + eyeVec);
	
	float NdotL		= dot ( normal, lightVec );
	float NdotH		= dot ( normal, halfVec );
	
	float clampedNdotL		= max ( NdotL, 0.0 );
	
	vec3 diff		= diffuse * clampedNdotL * lightIntensity;
	vec3 spec		= specular.xyz * specColor * pow ( max ( NdotH, 0.0 ), shininess ) * lightIntensity * clampedNdotL;
	
	return ((diffColor * (ambient + diff)) + spec);
}	

#endif