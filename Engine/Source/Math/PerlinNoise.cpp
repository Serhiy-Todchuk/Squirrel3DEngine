// PerlinNoise.cpp: implementation of the PerlinNoise class.
//
//////////////////////////////////////////////////////////////////////

#include "PerlinNoise.h"

namespace Squirrel {

namespace Math {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PerlinNoise::PerlinNoise(size_t octavesNum)
{
	mOctaves		= octavesNum;
	mPersistence	= cosf(sqrtf(2))*3.14f;
	mAmplitude		= 1.4f;
	mFrequency		= sqrtf(3.14f)*0.25f;
}

PerlinNoise::~PerlinNoise()
{
}

//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

void PerlinNoise::set(size_t octaves, float amlitude, float frequency, float persistence)
{
	mOctaves		= octaves;
	mPersistence	= persistence;
	mAmplitude		= amlitude;
	mFrequency		= frequency;
}
	
float PerlinNoise::smoothedNoise2D(float x, float y)
{
	float corners = (noise2D((int)x-1, (int)y-1) +
					 noise2D((int)x+1, (int)y-1) +
					 noise2D((int)x-1, (int)y+1) +
					 noise2D((int)x+1, (int)y+1) )	* 0.0625f;
	float sides   = (noise2D((int)x-1, (int)y) +
					 noise2D((int)x+1, (int)y) +
					 noise2D((int)x, (int)y-1) +
					 noise2D((int)x, (int)y+1) )	* 0.125f;
	float center  =  noise2D((int)x, (int)y)		* 0.25f;
	return corners + sides + center;
}

float PerlinNoise::compiledNoise2D(float x, float y)
{
	float int_X    = (float)int(x);
	float fractional_X = x - int_X;

	float int_Y    = (float)int(y);
	float fractional_Y = y - int_Y;

	float v1 = smoothedNoise2D(int_X,     int_Y);
	float v2 = smoothedNoise2D(int_X + 1, int_Y);
	float v3 = smoothedNoise2D(int_X,     int_Y + 1);
	float v4 = smoothedNoise2D(int_X + 1, int_Y + 1);

	float i1 = cosineInterpolate(v1 , v2 , fractional_X);
	float i2 = cosineInterpolate(v3 , v4 , fractional_X);

	return cosineInterpolate(i1 , i2 , fractional_Y);
}

float PerlinNoise::perlinNoise2D(float x, float y, float factor)
{
	float total = 0;
	
	//noise function is coherent, so we have to add factor of randomity
	x+= (factor);
	y+= (factor);
	
	float amplitude = mAmplitude;
	float frequency = mFrequency;
	
	// mOctaves - number of octaves, more octaves - better noise
	for(int i = 0; i < mOctaves; ++i)
	{
		total += compiledNoise2D(x * frequency, y * frequency) * amplitude;
		amplitude	*= mPersistence;
		frequency	*= 2;
	}
	
	//total=sqrt(total);
	//total=total*total;
	//total=sqrt(1.0f/total); 
	//total=cosf(total);
	//total=fabsf(total);
	
	return total;
}
	
byte * PerlinNoise::gen(size_t size, int randomSeed)
{
	srand(randomSeed);
	
	float fac =getRandomMinMax(getNextRandom()*PI*2,getNextRandom()*PI*3*100);
	
	return gen(size, randomSeed, fac);
}
	
byte * PerlinNoise::gen(size_t size, int randomSeed, float factor)
{
	if(size == 0)
		return NULL;

	byte * noise = new byte[size*size];
	
	srand(randomSeed);
	
	for(int i = 0; i < size; i++)
		for(int j = 0; j < size; j++)
			noise[ i*size+j ] = (byte)(perlinNoise2D(float(i), float(j), factor) * 255);

	return noise;
}

} //namespace Math {

} //namespace Squirrel {