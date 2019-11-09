#ifdef SQ_VERTEX_SHADER

#if defined(PARALLAX_MAPPING) && !defined(FOUR_TEXTURES)
varying	vec3 eyeVecTS;	//vector to eye in tangent space
#endif

#ifdef BUMP
varying mat3 tangentBasis;
#else
varying	vec3 normal;
#endif

//transformed vertex attribs

varying	vec2 texCoord;
varying	vec3 globalPos;

#if defined(FOUR_TEXTURES)
varying	vec4 mapsWeights;
#endif

//camera properties

uniform	vec3 eyePos;

//terrain specific uniforms

uniform float texCoordMult;

//matrices

uniform mat3 uNormalMatrix;
uniform mat4 uMVPMatrix;
uniform mat4 uModelViewMatrix;

//vertex attribs

attribute vec4 inPosition;
attribute vec3 inNormal;

void main(void)
{
#ifdef BUMP
    vec3 normal;
#endif
      
	vec4 pos	= uModelViewMatrix * inPosition;			// transformed point to world space
	normal		= normalize ( uNormalMatrix * inNormal );	// transformed normal
	globalPos	= pos.xyz;
	
#ifdef BUMP
    vec3 tangent    = cross(normal, vec3(0, 0, 1));
    vec3 binormal   = cross(tangent, normal);
	
	tangentBasis[0] = tangent;
	tangentBasis[1] = binormal;
	tangentBasis[2] = normal;
	
# if defined(PARALLAX_MAPPING) && !defined(FOUR_TEXTURES)
	eyeVecTS		= normalize ( eyePos - pos.xyz );			// world space vector to the eye
	eyeVecTS		= vec3 ( dot ( eyeVecTS, tangent ), dot ( eyeVecTS, normal ), dot ( eyeVecTS, binormal ) );
# endif
#endif

	gl_Position     = uMVPMatrix * inPosition;
	texCoord		= globalPos.xz * texCoordMult;
	
#if defined(FOUR_TEXTURES)
	float slope = 1.0 - normal.y;
	
	const float slopeTex2Start = 0.05;
	const float slopeTex2Fade = 0.04;	
	
	const float slopeTexStart = 0.09;
	const float slopeTexFade = 0.2;
	
	float slopeTexWeight = clamp((slope - slopeTexStart) / slopeTexFade, 0.0, 1.0);
	float slopeTex2Weight = clamp((slope - slopeTex2Start) / slopeTex2Fade, 0.0, 1.0);
	
	slopeTex2Weight = min(1.0 - slopeTexWeight, slopeTex2Weight);
	
	float middleTexWeight = 1.0 - (slopeTexWeight + slopeTex2Weight);
	
	const float lowTextStartHeight = -8.0;
	const float lowTexEndHeight = 8.0;
	const float lowTexFade = 0.5;
	
	const float highTexFade = 2.5;
	const float highTexHeight = 180.0;

	float height = globalPos.y;
	
	//float lowTexWeight = (height - lowTextStartHeight)
	
	float highFactor = clamp((height - highTexHeight) / highTexFade, 0.0, 1.0);
	
	float highTexWeight = middleTexWeight * highFactor;
	
	middleTexWeight = middleTexWeight * (1.0 - highFactor);
	
	mapsWeights.x = slopeTexWeight;
	mapsWeights.y = slopeTex2Weight;
	mapsWeights.z = middleTexWeight;
	mapsWeights.w = highTexWeight;
#endif
}

#endif
#ifdef SQ_FRAGMENT_SHADER

#ifdef BUMP
varying mat3 tangentBasis;
#else
varying	vec3 normal;
#endif

varying	vec2 texCoord;
varying	vec3 globalPos;

#if defined(FOUR_TEXTURES)
varying	vec4 mapsWeights;
#endif

//camera properties

uniform	vec3 eyePos;

//fog properties

uniform vec4 fogColor;
uniform float fogStart;
uniform float fogEnd;

uniform sampler2D decalMap;
#if defined(FOUR_TEXTURES)
	uniform sampler2D decalMap2;
	uniform sampler2D decalMap3;
	uniform sampler2D decalMap4;
	uniform vec4 specularValues;
#endif

#ifdef BUMP
uniform sampler2D normalHeightMap;
	#if defined(FOUR_TEXTURES)
		 uniform sampler2D normalHeightMap2;
		 uniform sampler2D normalHeightMap3;
		 uniform sampler2D normalHeightMap4;
	#endif
#endif

#ifdef BUMP_DIST
uniform float bumpMinDist;
uniform float bumpErpLength;
#endif

#ifdef CLIP
uniform vec4 uClipPlane;
#endif
 
#include <common/lighting.glsl>
		 
#ifdef SHADOW_MAP
# include <common/shadow.glsl>
#endif
	
#if defined(PARALLAX_MAPPING) && !defined(FOUR_TEXTURES)
varying	vec3 eyeVecTS;	//vector to eye in tangent space 
# include <common/parallax.glsl>
#endif

void main (void)
{
#ifdef CLIP
	if(dot(uClipPlane, vec4(globalPos, 1.0)) < 0.0)
		discard;
#endif
	
	float distanceFromCamera = gl_FragCoord.z / gl_FragCoord.w;//distance(eyePos, globalPos);
	
	vec2 tex = texCoord;
		 
#if defined(PARALLAX_MAPPING) && !defined(FOUR_TEXTURES)
	tex = parallaxMapping(tex, eyeVecTS, normalHeightMap);
#endif

	float lightIntensity;
	vec3 lightVec;
	computeLightVecAndIntensity(globalPos, lightVec, lightIntensity);

	vec4 color = texture2D ( decalMap, tex ).rgba;

	float specularValue = 1.0;

#if defined(FOUR_TEXTURES)
	color = color * mapsWeights.x;//first map
	if(mapsWeights.y > 0.01)
		color += texture2D ( decalMap2, tex ) * mapsWeights.y;//second map
	if(mapsWeights.z > 0.01)
		color += texture2D ( decalMap3, tex ) * mapsWeights.z;//third map
	if(mapsWeights.w > 0.01)
		color += texture2D ( decalMap4, tex ) * mapsWeights.w;//fourth map

	specularValue = mapsWeights.x * specularValues.x + mapsWeights.y * specularValues.y + mapsWeights.z * specularValues.z + mapsWeights.w * specularValues.w;
#endif
		 
#ifdef BUMP
	vec3 normal	= texture2D ( normalHeightMap, texCoord ).xyz * 2.0 - 1.0;
# if defined(FOUR_TEXTURES)
	normal = normal * mapsWeights.x;//first map
	if(mapsWeights.y > 0.01)
		normal += (texture2D ( normalHeightMap2, tex ).xyz * 2.0 - 1.0) * mapsWeights.y;//second map
	if(mapsWeights.z > 0.01)
		normal += (texture2D ( normalHeightMap3, tex ).xyz * 2.0 - 1.0) * mapsWeights.z;//third map
	if(mapsWeights.w > 0.01)
		normal += (texture2D ( normalHeightMap4, tex ).xyz * 2.0 - 1.0) * mapsWeights.w;//fourth map
# endif
	normal = tangentBasis * normalize( normal );
# ifdef BUMP_DIST
	float bumpInvValue = clamp((distanceFromCamera - bumpMinDist) / bumpErpLength, 0.0, 1.0);
	normal = lerp(normal, tangentBasis[2], bumpInvValue);
# endif
#endif

#ifdef SHADOW_MAP
	if(dot(normal, lightVec) > 0.0)
		lightIntensity *= clamp(shadowCoef(globalPos),0.3, 1.0);
	else
		lightIntensity *= 0.3;
#endif

	//perform lighting in world space to reduce bad tangent space artifacts
		 
	vec3 eyeVec		= normalize(eyePos - globalPos);
	vec3 frag		= performLighting(normal, eyeVec, lightVec, lightIntensity, color.xyz, vec3(specularValue));
	vec4 fragColor	= vec4 ( frag , color.a );

	float fog = clamp((distanceFromCamera - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
	fragColor = mix(fragColor, fogColor, fog * fog);

	gl_FragColor = fragColor;
}

#endif