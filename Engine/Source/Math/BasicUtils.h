// **************************************************************************
// Crazy Dreamer, 2010
// **************************************************************************

#pragma once

#include <float.h>	 
#include <math.h>
#include <ctime>
#include <iostream>

namespace Squirrel {

namespace Math {
    
//constants

#ifndef PI
	#define PI			3.1415926535897932384626433832795028841971693993751f
#endif
#define TWO_PI          6.28318530717958647692f
#define HALF_PI         1.57079632679489661923f
#define INV_PI			0.31830988618379067154f
#define EPSILON         0.0001f
#define EPSILON_SQUARED EPSILON*EPSILON
#define RAD2DEG         57.2957795130823208768f
#define DEG2RAD         0.01745329251994329577f
#define	EXP				2.71828182845904523536f	
#define	INVLOG2			3.32192809488736234787f									//1.0 / log10(2)
#define	LN2				0.693147180559945f										//ln(2)
#define	INVLN2			1.44269504089f											//1.0f / ln(2)
#define	INV3			0.33333333333333333333f									//1/3
#define	INV6			0.16666666666666666666f									//1/6
#define	INV7			0.14285714285714285714f									//1/7
#define	INV9			0.11111111111111111111f									//1/9
#define	INV255			0.00392156862745098039f									//1/255
#define	SQRT2			1.41421356237f											//sqrt(2)
#define	INVSQRT2		0.707106781188f											//1 / sqrt(2)
#define	SQRT3			1.73205080757f											//sqrt(3)
#define	INVSQRT3		0.577350269189f											//1 / sqrt(3)

#define POW_2_8			256
#define POW_2_16		65536
#define POW_2_24		16777216
#define POW_2_32		4294967296

#define _TRUNC_(a)	floorf(fabsf(a))
#define _FRAC_(a)	fabsf(a)-_TRUNC_(a)

inline int roundToBigger( float value )
{
	int intVal = (int)value;
	return value > intVal ? intVal + 1 : intVal;
}

inline int roundToSmaller( float value )
{
	return (int)value;
}

template<class T>
T sqr ( const T& value )
{
	return value * value;
}

template<class T>
T cube ( const T& value )
{
	return value * value * value;
}

template<class T>
T absValue ( const T& value )
{
	return value < 0 ? -value : value;
}
	
template<class T>
T minValue ( const T& value1, const T& value2 )
{
	return (value1 < value2) ? value1 : value2;
}

template<class T>
T maxValue ( const T& value1, const T& value2 )
{
	return (value1 > value2) ? value1 : value2;
}

template<class T>
T mixValue ( const T& value1, const T& value2, const float f )
{
	return value1*(1-f)+value2*f;
}

inline bool isFloatConsiderZero(float f, float tolerance = EPSILON)
{
	return fabsf(f)<tolerance;
}

/** Returns true if @a value1 and @a value2 are equal within @a tolerance*/
template<class T>
bool equals ( const T& value1, const T& value2, const T& tolerance = (T)EPSILON )
{
	return value1 - value2 < tolerance && value1 - value2 > -tolerance;
}

/** Retrieves the sign of a number.
@param val The number to check.
@return 1 if positive, -1 if negative. */
template <class T>
T sign ( const T& val )
{
	return ( val >= T ( 0 ) ) ? T ( 1 ) : T ( -1 );
}

inline float clamp2Pi(float in)
{
	float in_abs=fabsf(in);
	float sign=in<0.0f?-1.0f:1.0f;
	if(in_abs>TWO_PI) return sign*(in_abs-TWO_PI*floorf(in_abs/TWO_PI));
	return in;
}

template <class T>
inline T clamp(T x, T min, T max)
{ 
	return (x < min) ? min : (x > max) ? max : x;
}

inline int round(float f)
{
	return int(f + 0.5f);
}

inline float getRandomMinMax( float fMin, float fMax )
{
    float fRandNum = (float)rand () / RAND_MAX;
    return fMin + (fMax - fMin) * fRandNum;
}

inline float getNextRandom(){
	return (float)rand()/(RAND_MAX + 1);
}

inline int getClosest(int arg, int firstVal, int secondVal)
{
	int difference1 = abs(arg - firstVal);
	int difference2 = abs(arg - secondVal);
	return difference1 < difference2 ? firstVal : secondVal;
}

inline int repeat( int value, int length )
{
	if(value < 0)
	{
		return length + (value % length);
	}
	else
	{
		return value % length;
	}
}

inline bool isPowerOfTwo(int digit)
{
	double log2  = log(double(abs(digit)))/log(2.0);
  
	return int(log2) == log2;
}


inline int getClosestPowerOfTwo(int digit)
{
	if(!digit)	return 1;

	double log2  = log(double(abs(digit)))/log(2.0),
		flog2 = floor(log2),
		frac  = log2 - flog2;
  
	return (frac < 0.5) ? (int)pow(2.0, flog2) : (int)pow(2.0, flog2 + 1.0);
}

inline int getNextPowerOfTwo(int x)
{
    int i = x & (~x + 1);

    while (i < x)
        i <<= 1;

    return i;
}

inline float fsqrt(float x)
{
#ifdef _WIN32
  __asm
  {
    fld x;
    fsqrt;
    fstp x;
  }
  return x;
#else
    return sqrtf(x);
#endif
}

inline float fcos(float x)
{
#ifdef _WIN32
  __asm
  {
    fld x;
    fcos;
    fstp x;
  }
  return x;
#else
    return cosf(x);
#endif
}

inline float fsin(float x)
{
#ifdef _WIN32
  __asm
  {
    fld x;
    fsin;
    fstp x;
  }
  return x;
#else
    return sinf(x);
#endif
}

inline void fsincos(float a, float &s, float &c) 
{ 
#ifdef _WIN32
  __asm
  { 
    fld a
    fsincos
    fstp dword ptr [c]
    fstp dword ptr [s]
  }
#else
    c = cosf(a);
    s = sinf(a);
#endif
}

inline float CoTan(float x)
{
#ifdef _WIN32
	x;								// Just for this shit "unreferenced formal parameter" warning
    __asm
	{
		fld      dword ptr [x]
		fptan
		fdivrp   st(1), st
	}
#else
    return 1.0f/tanf(x);
#endif
}

//! Fast square root for floating-point values.
inline float fastSqrt(float square)
{
#ifdef _WIN32
	float retval;
	__asm {
			mov             eax, square
			sub             eax, 0x3F800000
			sar             eax, 1
			add             eax, 0x3F800000
			mov             [retval], eax
	}
	return retval;
#else
    return sqrtf(square);
#endif
}

//! Computes 1.0f / sqrtf(x). Comes from NVIDIA.
inline float invSqrtNV(const float& x)
{
#ifdef _WIN32
	unsigned int tmp = (unsigned int(0x3f800000 << 1) + 0x3f800000 - *(unsigned int*)&x) >> 1;
	float y = *(float*)&tmp;
	return y * (1.47f - 0.47f * x * y * y);
#else
    return 1.0f / sqrtf(x);
#endif
}

inline float fastSqrt2(float f)
{
	unsigned int y = ( ( (signed int&)f - 0x3f800000 ) >> 1 ) + 0x3f800000;
	// Iteration...?
	// (float&)y = (3.0f - ((float&)y * (float&)y) / f) * (float&)y * 0.5f;
	// Result
	return (float&)y;
}

inline float fastCos(const float x)
{
	// assert:  0 <= fT <= PI/2
	// maximum absolute error = 1.1880e-03
	// speedup = 2.14
	float x_sqr = x*x;
	float res = float(3.705e-02);
	res *= x_sqr;
	res -= float(4.967e-01);
	res *= x_sqr;
	res += 1;
	return res;
}

inline float fastCos2(const float x)
{
	// assert:  0 <= fT <= PI/2
	// maximum absolute error = 2.3082e-09
	// speedup = 1.47
	float x_sqr = x*x;
	float res = float(-2.605e-07);
	res *= x_sqr;
	res += float(2.47609e-05);
	res *= x_sqr;
	res -= float(1.3888397e-03);
	res *= x_sqr;
	res += float(4.16666418e-02);
	res *= x_sqr;
	res -= float(4.999999963e-01);
	res *= x_sqr;
	res += 1;
	return res;
}

#define IR(x)					((unsigned int&)(x))
//! Is the float valid ?
inline bool isNAN(float value)				{ return (IR(value)&0x7f800000) == 0x7f800000;	}
inline bool isIndeterminate(float value)	{ return IR(value) == 0xffc00000;				}
inline bool isPlusInf(float value)			{ return IR(value) == 0x7f800000;				}
inline bool isMinusInf(float value)			{ return IR(value) == 0xff800000;				}
inline bool isValidFloat(float value)
{
	if(isNAN(value))			return false;
	if(isIndeterminate(value))	return false;
	if(isPlusInf(value))		return false;
	if(isMinusInf(value))		return false;
	return true;
}

template <class T>
inline T lerp(T a, T b, float frac)
{
	return (a * (1 - frac)) + (b * frac);
}

inline float cosineInterpolate(float a, float b, float frac)
{
	float ft = frac * PI;
	float f = (1 - fcos(ft)) * 0.5f;
	return lerp(a, b, f);  
}

inline float jamesLongInterpolate(float a, float b, float frac)
{
	float fac1 = 3*powf(1-frac, 2) - 2*powf(1-frac,3);
	float fac2 = 3*powf(frac, 2) - 2*powf(frac, 3);
	return a*fac1 + b*fac2;// James Long- использовал этот метод в своей статье
}

inline float noise2D(int x, int y)
{
	int n = x + y * 57;
	n = (n<<13) ^ n;
	return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

} //namespace Math {

} //namespace Squirrel {



