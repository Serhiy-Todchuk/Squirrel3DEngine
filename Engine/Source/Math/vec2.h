#pragma once

#include "BasicUtils.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

class SQMATH_API vec2
{
	public:
	
	float x, y;
	
	inline vec2(void) {}
	
	inline vec2(float _x, float _y)
	{
		x = _x, y = _y;
	}
	inline vec2(float theta)
	{
		x = fcos(theta), y = fsin(theta);
	}
	inline void zero(void)
	{
		x = y = 0.0f;
	}
	inline bool isConsiderZero(float tolerance = EPSILON) const
	{
		return isFloatConsiderZero(x, tolerance) && isFloatConsiderZero(y, tolerance);
	}
	inline bool operator ==(const vec2& a) const
	{
		return x == a.x && y == a.y;
	}
	inline bool operator !=(const vec2& a) const
	{
		return x != a.x || y != a.y;
	}
	inline vec2 operator +(const vec2& a) const
	{
		return vec2(x + a.x, y + a.y);
	}
	inline vec2 operator -(const vec2& a) const
	{
		return vec2(x - a.x, y - a.y);
	}
	inline float operator *(const vec2& a) const
	{
		return x*a.x + y*a.y;
	}
	inline vec2 operator *(const float a) const
	{
		return vec2(x*a, y*a);
	}
	inline vec2 operator /(const float a) const
	{
		return (*this) * (1.0f / a);
	}
	inline float operator ^(const vec2& a) const
	{
		return y*a.x - x*a.y;
	}
	inline const vec2& operator +=(const vec2& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const vec2& operator -=(const vec2& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const vec2& operator *=(const float a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const vec2& operator /=(const float a)
	{
		(*this) = (*this) / a; return (*this);
	}
	inline float operator [](int i) const
	{
		return ((float*)this)[i];
	}
	inline float& operator [](int i)
	{
		return ((float*)this)[i];
	}
	inline float lenSquared(void) const
	{
		return (x*x + y*y);
	}
	inline float len(void) const
	{
		return fsqrt(lenSquared());
	}
	inline vec2 normalized(void) const
	{
		return (*this) / len();
	}
	inline void normalize(void)
	{
		(*this) /= len();
	}

	static vec2 Zero(void) { return vec2(0, 0); }
};

} //namespace Math {

} //namespace Squirrel {