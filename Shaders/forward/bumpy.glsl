#ifdef SQ_VERTEX_SHADER

#define EPS   0.001

//lighting vectors

#ifdef PARALLAX_MAPPING
varying	vec3 eyeVecTS;	//vector to eye in tangent space
#endif

varying mat3 tangentBasis;

//transformed vertex attribs

varying	vec2 texCoord;
varying	vec3 globalPos;

//camera properties

uniform	vec3 eyePos;

//matrices

uniform mat3 uNormalMatrix;
uniform mat4 uMVPMatrix;
uniform mat4 uModelViewMatrix;

//vertex attribs

attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexcoord;
attribute vec4 inTangentBinormal;

#ifdef SKINNING

// 3x4 matrix, passed as vec4's for compatibility with GL 2.0
// Support 55 bones ie 55*3 = 165
uniform vec4 bones[165];

attribute vec4 inBoneIndices;
attribute vec4 inBoneWeights4;

mat4 getBoneTransform ( float index )
{
	int intIndex = int(index * 3.0);
	mat4 boneTransform;
	boneTransform[0] = bones[ intIndex + 0 ];
	boneTransform[1] = bones[ intIndex + 1 ];
	boneTransform[2] = bones[ intIndex + 2 ];
	boneTransform[3] = vec4( 0.0 );
    return boneTransform;
}

#endif

void main(void)
{
	vec3 inTangent = inTangentBinormal.xyz;
	float binormalMultiplier = inTangentBinormal.w;

#ifndef SKINNING

	vec3 pos = inPosition.xyz;
	vec3 nor = inNormal;
	vec3 tan = inTangent;

#else

	vec3 pos = vec3(0.0);
	vec3 nor = vec3(0.0);
	vec3 tan = vec3(0.0);

    vec4    weights = inBoneWeights4;        // weights for 4 bones
    vec4    indices = inBoneIndices;        // indices for 4 bones
    mat4	boneTransform;
    mat3	boneRotation;

	if ( weights.x > EPS )                      // process 1st bone
    {
        boneTransform	= getBoneTransform( indices.x );
		boneRotation	= mat3(boneTransform[0].xyz, boneTransform[1].xyz, boneTransform[2].xyz);
		pos  += ((inPosition * boneTransform) * weights.x).xyz;
		nor  += (inNormal * boneRotation) * weights.x;
		tan  += (inTangent * boneRotation) * weights.x;
    }

	if ( weights.y > EPS )                      // process 2nd bone
    {
        boneTransform	= getBoneTransform( indices.y );
		boneRotation	= mat3(boneTransform[0].xyz, boneTransform[1].xyz, boneTransform[2].xyz);
		pos  += ((inPosition * boneTransform) * weights.y).xyz;
		nor  += (inNormal * boneRotation) * weights.y;
		tan  += (inTangent * boneRotation) * weights.y;
    }

	if ( weights.z > EPS )                      // process 3rd bone
    {
        boneTransform	= getBoneTransform( indices.z );
		boneRotation	= mat3(boneTransform[0].xyz, boneTransform[1].xyz, boneTransform[2].xyz);
		pos  += ((inPosition * boneTransform) * weights.z).xyz;
		nor  += (inNormal * boneRotation) * weights.z;
		tan  += (inTangent * boneRotation) * weights.z;
    }	

	if ( weights.w > EPS )                      // process 4th bone
    {
		boneTransform	= getBoneTransform( indices.w );
		boneRotation	= mat3(boneTransform[0].xyz, boneTransform[1].xyz, boneTransform[2].xyz);
		pos  += ((inPosition * boneTransform) * weights.w).xyz;
		nor  += (inNormal * boneRotation) * weights.w;
		tan  += (inTangent * boneRotation) * weights.w;
    }	
	
#endif

	vec4 posOS = vec4 ( pos, 1.0 );			//object space pos
	vec4 posWS = uModelViewMatrix * posOS;	//world space pos
	globalPos = vec3(posWS);		// transformed point to world space
		
	vec3 normal		= normalize( uNormalMatrix * nor );
    vec3 tangent    = normalize( uNormalMatrix * tan );
    vec3 binormal   = normalize( uNormalMatrix * (cross( nor, tan ) * binormalMultiplier) );
	
#ifdef PARALLAX_MAPPING
	eyeVecTS		= normalize ( eyePos - globalPos );			// world space vector to the eye
	eyeVecTS		= vec3 ( dot ( eyeVecTS, tangent ), dot ( eyeVecTS, normal ), dot ( eyeVecTS, binormal ) );
#endif
	
	tangentBasis[0] = tangent;
	tangentBasis[1] = binormal;
	tangentBasis[2] = normal;
	
	gl_Position     = uMVPMatrix * posOS;
	texCoord		= inTexcoord;
}

#endif
#ifdef SQ_FRAGMENT_SHADER

#ifdef PARALLAX_MAPPING
varying	vec3 eyeVecTS;	//vector to eye in tangent space
#endif

varying mat3 tangentBasis;		 
		 
varying	vec2 texCoord;
varying	vec3 globalPos;

//camera properties

uniform	vec3 eyePos;

#ifndef NOTEXTURES

uniform sampler2D decalMap;
uniform sampler2D normalHeightMap;

# ifdef SPECULAR_MAP
uniform sampler2D specularMap;
# endif

# ifdef DETAIL_MAP
uniform sampler2D detailMap;
# endif

#endif

uniform float uAlphaTest;

#ifdef CLIP
uniform vec4 uClipPlane;
#endif

#ifdef EMISSION
uniform vec3 emissionColor;
# ifdef EMISSION_MAP
uniform sampler2D emissionMap;
# endif
#endif

#include <common/lighting.glsl>		 
		 
#ifdef SHADOW_MAP
# include <common/shadow.glsl>
#endif
		 
#ifdef PARALLAX_MAPPING
# include <common/parallax.glsl>
#endif

void main (void)
{
#ifdef CLIP
	if(dot(uClipPlane, vec4(globalPos, 1.0)) < 0)
		discard;
#endif

    vec2 tex = texCoord;
    
#ifdef PARALLAX_MAPPING
	tex = parallaxMapping(tex, eyeVecTS, normalHeightMap);
#endif
	
#ifndef NOTEXTURES
	vec4 color		= texture2D ( decalMap, tex ).rgba;
	vec4 nhmFetch	= texture2D ( normalHeightMap, tex );
	vec3 mapNormal	= nhmFetch.xyz * 2.0 - 1.0;
	//mapNormal.z *= 0.3;
	vec3 normal		= tangentBasis * normalize(mapNormal);
# ifdef DETAIL_MAP
	color.rgb *= texture2D ( detailMap, tex ).rgb;
# endif
#else
	vec4 color		= vec4(1.0, 1.0, 1.0, 1.0);
	vec3 normal		= tangentBasis[2];
#endif

	if(color.a < uAlphaTest)
		discard;

	float lightIntensity;
	vec3 lightVec;
	computeLightVecAndIntensity(globalPos, lightVec, lightIntensity);

	vec3 specColor = vec3(1,1,1);

#ifdef SPECULAR_MAP
	specColor = texture2D ( specularMap, tex ).rgb;
#endif

#ifdef SHADOW_MAP
	if(dot(normal, lightVec) > 0.0)
		lightIntensity *= clamp(shadowCoef(globalPos), 0.3, 1.0);
#endif

	//perform lighting in world space to reduce bad tangent space artifacts
		 
	vec3 eyeVec		= normalize(eyePos - globalPos);	
	vec3 frag		= performLighting(normal, eyeVec, lightVec, lightIntensity, color.rgb, specColor);

#ifdef EMISSION
	vec3 fragEmission = emissionColor;
# ifdef EMISSION_MAP
	fragEmission *= texture2D(emissionMap, tex).rgb;
# endif
	frag += fragEmission;
#endif
	
	gl_FragColor	= vec4 ( frag, color.a );
}

#endif