#ifdef SQ_VERTEX_SHADER

#define EPS   0.001
   
attribute vec4 inPosition;  
 
#ifdef TEXTURE_ALPHA
	attribute vec2 inTexcoord;
	varying	vec2 texCoord;
#endif

uniform mat4 uMVPMatrix;

#ifdef WRITE_DISTANCE
uniform mat4 uModelViewMatrix;

uniform vec4 lightPos;

varying vec3 depthVec;
#endif

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
#ifndef SKINNING

	vec3 pos = inPosition.xyz;

#else

	vec3 pos = vec3(0.0);

    vec4    weights = inBoneWeights4;        // weights for 4 bones
    vec4    indices = inBoneIndices;        // indices for 4 bones
    mat4	boneTransform;
 
	if ( weights.x > EPS )                      // process 1st bone
    {
        boneTransform	= getBoneTransform( indices.x );
		pos  += ((inPosition * boneTransform) * weights.x).xyz;
    }

	if ( weights.y > EPS )                      // process 2nd bone
    {
        boneTransform	= getBoneTransform( indices.y );
		pos  += ((inPosition * boneTransform) * weights.y).xyz;
    }

	if ( weights.z > EPS )                      // process 3rd bone
    {
        boneTransform	= getBoneTransform( indices.z );
		pos  += ((inPosition * boneTransform) * weights.z).xyz;
    }	

	if ( weights.w > EPS )                      // process 4th bone
    {
		boneTransform	= getBoneTransform( indices.w );
		pos  += ((inPosition * boneTransform) * weights.w).xyz;
    }	
		
#endif
	
#ifdef WRITE_DISTANCE
	vec4 worldPos = uModelViewMatrix * vec4 ( pos, 1.0 );
	
	depthVec = (worldPos.xyz - lightPos.xyz) / lightPos.w;
#endif

#ifdef TEXTURE_ALPHA
	texCoord		= inTexcoord;
#endif

	gl_Position     = uMVPMatrix * vec4 ( pos, 1.0 );
}

#endif
#ifdef SQ_FRAGMENT_SHADER

#ifdef WRITE_DISTANCE
	varying vec3 depthVec;
#endif

#ifdef TEXTURE_ALPHA

varying	vec2 texCoord;
uniform sampler2D decalMap;
uniform float uAlphaTest;
      
#endif
      
void main (void)
{
#ifdef TEXTURE_ALPHA
    vec4 color		= texture2D ( decalMap, texCoord ).rgba;
      
    if(color.a < uAlphaTest)
        discard;
#endif

#ifdef WRITE_DISTANCE
	float vDistance = length( depthVec );
	gl_FragDepth = vDistance;
	gl_FragColor = vec4 ( vDistance, vDistance, vDistance, 1.0 );
#else
	gl_FragColor = vec4 ( gl_FragCoord.zzz / gl_FragCoord.w, 1.0 );
#endif
}

#endif