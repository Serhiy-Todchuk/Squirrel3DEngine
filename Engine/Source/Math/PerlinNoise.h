// PerlinNoise.h: interface for the PerlinNoise class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "macros.h"
#include <Common/types.h>
#include "vec3.h"

namespace Squirrel {

namespace Math {
	
class SQMATH_API PerlinNoise  
{
public:
	const static int DEFAULT_OCTAVES_NUM = 8;
	
public:
	PerlinNoise(size_t octavesNum = DEFAULT_OCTAVES_NUM);
	virtual ~PerlinNoise();
	
	void set(size_t octaves, float amlitude, float frequency, float persistance);

	float perlinNoise2D(float x, float y, float factor);
	float compiledNoise2D(float x, float y);
	float smoothedNoise2D(float x, float y);
	
	byte * gen(size_t size, int randomSeed);
	byte * gen(size_t size, int randomSeed, float factor);
	
private:
	
	float mPersistence;
	float mFrequency;
	float mAmplitude;
	size_t	mOctaves;
};

} //namespace Math {

} //namespace Squirrel {
