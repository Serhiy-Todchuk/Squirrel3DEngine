#pragma once

#include "BasicUtils.h"
#include "vec2.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

// ****************************
// Two-Dimensional Matrix Class
// ****************************

class SQMATH_API mat2
{
public:
	
	vec2 x, y;
	
	inline mat2(void) {}
	
	inline mat2(float m11, float m12,
	            float m21, float m22)
	{
		x = vec2(m11, m12);
		y = vec2(m21, m22);
	}
	inline mat2(const vec2& mx, const vec2& my)
	{
		x = mx, y = my;
	}
	inline void identity(void)
	{
		x = vec2(1.0f, 0.0f);
		y = vec2(0.0f, 1.0f);
	}
	inline void zero(void)
	{
		x.zero();
		y.zero();
	}
	inline const bool operator ==(const mat2& a) const
	{
		return x == a.x && y == a.y;
	}
	inline const bool operator !=(const mat2& a) const
	{
		return x != a.x || y != a.y;
	}
	inline mat2 operator +(const mat2& a) const
	{
		return mat2(x + a.x, y + a.y);
	}
	inline mat2 operator -(const mat2& a) const
	{
		return mat2(x - a.x, y - a.y);
	}
	inline mat2 operator *(const mat2& a) const
	{
		mat2 tmp = a.transposed();
		
		return mat2(tmp*x, tmp*y);
	}
	inline vec2 operator *(const vec2& a) const
	{
		return vec2(x*a, y*a);
	}
	inline mat2 operator *(const float a) const
	{
		return mat2(x*a, y*a);
	}
	inline mat2 operator /(const float a) const
	{
		return (*this) * (1.0f / a);
	}
	inline const mat2& operator +=(const mat2& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const mat2& operator -=(const mat2& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const mat2& operator *=(const mat2& a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const mat2& operator *=(const float a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const mat2& operator /=(const float a)
	{
		(*this) = (*this) / a; return (*this);
	}
	inline const vec2& operator [](int i) const
	{
		return ((vec2*)this)[i];
	}
	inline vec2& operator [](int i)
	{
		return ((vec2*)this)[i];
	}

	float det(void) const;
	mat2 transposed(void) const;
	mat2 inverse(void) const;

	static inline mat2 Identity() { return mat2(1,0,0,1); }
	static mat2 Rot(float theta);
};

} //namespace Math {

} //namespace Squirrel {
