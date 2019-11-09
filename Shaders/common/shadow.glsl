
#ifdef POINT_LIGHT

uniform samplerCube shadowMap;
uniform float shadowOffset;

float shadowCoef(vec3 fragPos)
{
	vec3 dir = fragPos - lightPos;
	float len = length( dir );
	dir /= len;
	
	len /= lightRad;
	
	len -= 0.01;
	
	//TODO: fix it
	dir.z = -dir.z;
	dir.y = -dir.y;

#if defined ( SHADOW_PCF_32TAP_RANDOM )
	//Poisson disk filter
	const int nsamples = 32;
	const vec3 offsets[nsamples] = vec3[](
										  vec3(-0.975402, -0.0711386, 0.000000),
										  vec3(-0.920347, -0.4114200, 0.000000),
										  vec3(-0.883908,  0.2178720, 0.000000),
										  vec3(-0.884518,  0.5680410, 0.000000),
										  vec3(-0.811945,  0.9052100, 0.000000),
										  vec3(-0.792474, -0.7799620, 0.000000),
										  vec3(-0.614856,  0.3865780, 0.000000),
										  vec3(-0.580859, -0.2087770, 0.000000),
										  vec3(-0.537950,  0.7166660, 0.000000),
										  vec3(-0.515427,  0.0899991, 0.000000),
										  vec3(-0.454634, -0.7079380, 0.000000),
										  vec3(-0.420942,  0.9912720, 0.000000),
										  vec3(-0.261147,  0.5884880, 0.000000),
										  vec3(-0.211219,  0.1148410, 0.000000),
										  vec3(-0.146336, -0.2591940, 0.000000),
										  vec3(-0.139439, -0.8886680, 0.000000),
										  vec3( 0.011688,  0.3263950, 0.000000),
										  vec3( 0.038056,  0.6254770, 0.000000),
										  vec3( 0.062593, -0.5085300, 0.000000),
										  vec3( 0.125584,  0.0469069, 0.000000),
										  vec3( 0.169469, -0.9972530, 0.000000),
										  vec3( 0.320597,  0.2910550, 0.000000),
										  vec3( 0.359172, -0.6337170, 0.000000),
										  vec3( 0.435713, -0.2508320, 0.000000),
										  vec3( 0.507797, -0.9165620, 0.000000),
										  vec3( 0.545763,  0.7302160, 0.000000),
										  vec3( 0.568590,  0.1165500, 0.000000),
										  vec3( 0.743156, -0.5051730, 0.000000),
										  vec3( 0.736442, -0.1897340, 0.000000),
										  vec3( 0.843562,  0.3570360, 0.000000),
										  vec3( 0.865413,  0.7637260, 0.000000),
										  vec3( 0.872005, -0.9270000, 0.000000)
										  );
	float ret = 0.0;
	for(int i=0; i<nsamples; i++)
	{
	    vec3 shadow_lookup = dir + offsets[i] * shadowOffset;
	    ret += textureCube( shadowMap, shadow_lookup).x > len ? 0.125 : 0.0;
	}
#elif defined ( SHADOW_PCF_8TAP_RANDOM )
	//Poisson disk filter
	const int nsamples = 8;
	const vec3 offsets[nsamples] = vec3[](	vec3( 0.000000,  0.000000,  0.000000),
										vec3( 0.079821,  0.165750, -0.558746),
										vec3(-0.331500,  0.159642, -0.994499),
										vec3(-0.239463, -0.497250,  0.399104),
										vec3( 0.662999, -0.319284,  0.159642),
										vec3( 0.399104,  0.828749, -0.239463),
										vec3(-0.994499,  0.478925, -0.331500),
										vec3(-0.558746, -1.160249,  0.079821) );
	float ret = 0.0;
	for(int i=0; i<nsamples; i++)
	{
	    vec3 shadow_lookup = dir + offsets[i] * shadowOffset;
	    ret += textureCube( shadowMap, shadow_lookup).x > len ? 0.125 : 0.0;
	}
#elif defined ( SHADOW_PCF_3x3 ) || defined ( SHADOW_PCF_GAUSSIAN3x3 )
	// Simple 3x3 filter
	const float tapWeight = 1.0/9.0;
	float ret = textureCube(textureCube, dir).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3( -1.0, -1.0, 0.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3( -1.0,  0.0, 1.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3( -1.0,  1.0, 1.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3(  0.0, -1.0, 1.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3(  0.0,  1.0,-1.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3(  1.0, -1.0,-1.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3(  1.0,  0.0,-1.0) * shadowOffset).x > len ? tapWeight : 0.0;
	ret += textureCube(textureCube, dir + vec3(  1.0,  1.0, 0.0) * shadowOffset).x > len ? tapWeight : 0.0;
#elif defined ( SHADOW_PCF_5TAP )
	// 5 tap filter
	float ret = textureCube(shadowMap, dir).x > len ? 0.333 : 0.0;
	ret += textureCube(shadowMap, dir + vec3( -1.0,  1.0, 1.0) * shadowOffset).x > len ? 0.166 : 0.0;
	ret += textureCube(shadowMap, dir + vec3(  1.0, -1.0, 1.0) * shadowOffset).x > len ? 0.166 : 0.0;
	ret += textureCube(shadowMap, dir + vec3( -1.0,  1.0,-1.0) * shadowOffset).x > len ? 0.166 : 0.0;
	ret += textureCube(shadowMap, dir + vec3(  1.0, -1.0,-1.0) * shadowOffset).x > len ? 0.166 : 0.0;
#else
	float ret = textureCube(shadowMap, dir).x > len ? 1.0 : 0.0;
#endif
	return ret;
}

#else
uniform sampler2DShadow shadowMap;
				  
#ifdef SHADOW_SPLITS
uniform vec4 shadowOffset;
uniform vec3 shadowSplitDistances;
uniform sampler2DShadow shadowMapX;
uniform sampler2DShadow shadowMapXX;
uniform sampler2DShadow shadowMapXXX;
uniform mat4 uShadowMatrices[SHADOW_SPLITS];
#else
uniform float shadowOffset;
uniform mat4 uShadowMatrix;
#endif
				  
float shadowCoef(vec3 shadow_coord, sampler2DShadow shadow_map, float shadow_offset)
{	
#if defined ( SHADOW_PCF_32TAP_RANDOM )
	//Poisson disk filter
	const int nsamples = 32;
	const vec3 offsets[nsamples] = vec3[](vec3(-0.975402, -0.0711386, 0.000000),
										  vec3(-0.920347, -0.4114200, 0.000000),
										  vec3(-0.883908,  0.2178720, 0.000000),
										  vec3(-0.884518,  0.5680410, 0.000000),
										  vec3(-0.811945,  0.9052100, 0.000000),
										  vec3(-0.792474, -0.7799620, 0.000000),
										  vec3(-0.614856,  0.3865780, 0.000000),
										  vec3(-0.580859, -0.2087770, 0.000000),
										  vec3(-0.537950,  0.7166660, 0.000000),
										  vec3(-0.515427,  0.0899991, 0.000000),
										  vec3(-0.454634, -0.7079380, 0.000000),
										  vec3(-0.420942,  0.9912720, 0.000000),
										  vec3(-0.261147,  0.5884880, 0.000000),
										  vec3(-0.211219,  0.1148410, 0.000000),
										  vec3(-0.146336, -0.2591940, 0.000000),
										  vec3(-0.139439, -0.8886680, 0.000000),
										  vec3( 0.011688,  0.3263950, 0.000000),
										  vec3( 0.038056,  0.6254770, 0.000000),
										  vec3( 0.062593, -0.5085300, 0.000000),
										  vec3( 0.125584,  0.0469069, 0.000000),
										  vec3( 0.169469, -0.9972530, 0.000000),
										  vec3( 0.320597,  0.2910550, 0.000000),
										  vec3( 0.359172, -0.6337170, 0.000000),
										  vec3( 0.435713, -0.2508320, 0.000000),
										  vec3( 0.507797, -0.9165620, 0.000000),
										  vec3( 0.545763,  0.7302160, 0.000000),
										  vec3( 0.568590,  0.1165500, 0.000000),
										  vec3( 0.743156, -0.5051730, 0.000000),
										  vec3( 0.736442, -0.1897340, 0.000000),
										  vec3( 0.843562,  0.3570360, 0.000000),
										  vec3( 0.865413,  0.7637260, 0.000000),
										  vec3( 0.872005, -0.9270000, 0.000000)
										  );
	float ret = 0.0;
	for(int i=0; i < 12; i++)
	{
	    vec3 shadow_lookup = shadow_coord + offsets[i] * shadow_offset; //scale the offsets to the texture size, and make them twice as large to cover a larger radius
	    ret += shadow2D( shadow_map, shadow_lookup).x * 0.03125;
	}
#elif defined ( SHADOW_PCF_8TAP_RANDOM )
	//Poisson disk filter
	const int nsamples = 8;
	const vec3 offsets[nsamples] = vec3[](vec3( 0.000000,  0.000000, 0.0),
										  vec3( 0.079821,  0.165750, 0.0),
										  vec3(-0.331500,  0.159642, 0.0),
										  vec3(-0.239463, -0.497250, 0.0),
										  vec3( 0.662999, -0.319284, 0.0),
										  vec3( 0.399104,  0.828749, 0.0),
										  vec3(-0.994499,  0.478925, 0.0),
										  vec3(-0.558746, -1.160249, 0.0)
										  );
	float ret = 0.0;
	for(int i=0; i<nsamples; i++)
	{
	    vec3 shadow_lookup = shadow_coord + offsets[i] * shadow_offset; //scale the offsets to the texture size, and make them twice as large to cover a larger radius
	    ret += shadow2D( shadow_map, shadow_lookup).x * 0.125 ;
	}
#elif defined ( SHADOW_PCF_GAUSSIAN3x3 )
	// Gaussian 3x3 filter
	float ret = shadow2D(shadow_map, shadow_coord).x * 0.25;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0, -1.0, 0.0) * shadow_offset).x * 0.0625;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0,  0.0, 0.0) * shadow_offset).x * 0.125;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0,  1.0, 0.0) * shadow_offset).x * 0.0625;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  0.0, -1.0, 0.0) * shadow_offset).x * 0.125;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  0.0,  1.0, 0.0) * shadow_offset).x * 0.125;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0, -1.0, 0.0) * shadow_offset).x * 0.0625;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0,  0.0, 0.0) * shadow_offset).x * 0.125;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0,  1.0, 0.0) * shadow_offset).x * 0.0625;
#elif defined ( SHADOW_PCF_3x3 )
	// Simple 3x3 filter
	const float tapWeight = 1.0/9.0;
	float ret = shadow2D(shadow_map, shadow_coord).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0, -1.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0,  0.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0,  1.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  0.0, -1.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  0.0,  1.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0, -1.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0,  0.0, 0.0) * shadow_offset).x * tapWeight;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0,  1.0, 0.0) * shadow_offset).x * tapWeight;
#elif defined ( SHADOW_PCF_5TAP )
	// 5 tap filter
	float ret = shadow2D(shadow_map, shadow_coord).x * 0.333;
	ret += shadow2D(shadow_map, shadow_coord + vec3( -1.0,  0.0, 0.0) * shadow_offset).x * 0.166;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  0.0, -1.0, 0.0) * shadow_offset).x * 0.166;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  0.0,  1.0, 0.0) * shadow_offset).x * 0.166;
	ret += shadow2D(shadow_map, shadow_coord + vec3(  1.0,  0.0, 0.0) * shadow_offset).x * 0.166;
#else
	float ret = shadow2D(shadow_map, shadow_coord).x;	
#endif

	return ret;
}

float shadowCoef(vec3 fragPos)
{
#ifdef SHADOW_SPLITS

	float fragDepth = gl_FragCoord.z / gl_FragCoord.w;

# if SHADOW_SPLITS > 3
	if(fragDepth >= shadowSplitDistances.z)
	{
		vec4 shadow_coord = uShadowMatrices[3] * vec4(fragPos, 1.0);
		return shadowCoef(shadow_coord.xyz / shadow_coord.w, shadowMapXXX, shadowOffset.w);
	}
# endif
# if SHADOW_SPLITS > 2
	if(fragDepth >= shadowSplitDistances.y)
	{
		vec4 shadow_coord = uShadowMatrices[2] * vec4(fragPos, 1.0);
		return shadowCoef(shadow_coord.xyz / shadow_coord.w, shadowMapXX, shadowOffset.z);
	}
# endif
# if SHADOW_SPLITS > 1
	if(fragDepth >= shadowSplitDistances.x)
	{
		vec4 shadow_coord = uShadowMatrices[1] * vec4(fragPos, 1.0);
		return shadowCoef(shadow_coord.xyz / shadow_coord.w, shadowMapX, shadowOffset.y);
	}
# endif

	vec4 shadow_coord = uShadowMatrices[0] * vec4(fragPos, 1.0);
	return shadowCoef(shadow_coord.xyz / shadow_coord.w, shadowMap, shadowOffset.x);

#else
	// transform this fragment's position from view space to scaled light clip space
	// such that the xy coordinates are in [0;1]
	// note there is no need to divide by w for othogonal light sources
	vec4 shadow_coord = uShadowMatrix * vec4(fragPos, 1.0);
	return shadowCoef(shadow_coord.xyz / shadow_coord.w, shadowMap, shadowOffset);
#endif	
}

#endif