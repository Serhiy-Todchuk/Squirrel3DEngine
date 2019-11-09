#pragma once

#include "mat2.h"
#include "vec3.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

// ******************************
// Three-Dimensional Matrix Class
// ******************************

class SQMATH_API mat3
{
	public:
	
	vec3 x, y, z;
	
	inline mat3(void) {}
	
	inline mat3(float m11, float m12, float m13,
	            float m21, float m22, float m23,
	            float m31, float m32, float m33)
	{
		x = vec3(m11, m12, m13);
		y = vec3(m21, m22, m23);
		z = vec3(m31, m32, m33);
	}
	inline mat3(const vec3& mx, const vec3& my, const vec3& mz)
	{
		x = mx, y = my, z = mz;
	}
	inline void identity(void)
	{
		x = vec3(1.0f, 0.0f, 0.0f);
		y = vec3(0.0f, 1.0f, 0.0f);
		z = vec3(0.0f, 0.0f, 1.0f);
	}

	inline void zero(void)
	{
		x.zero();
		y.zero();
		z.zero();
	}
	inline const bool operator ==(const mat3& a) const
	{
		return x == a.x && y == a.y && z == a.z;
	}
	inline const bool operator !=(const mat3& a) const
	{
		return x != a.x || y != a.y || z != a.z;
	}
	inline mat3 operator +(const mat3& a) const
	{
		return mat3(x + a.x, y + a.y, z + a.z);
	}
	inline mat3 operator +(const mat2& a) const
	{
		return mat3(x + a.x, y + a.y, z);
	}
	inline mat3 operator -(const mat3& a) const
	{
		return mat3(x - a.x, y - a.y, z - a.z);
	}
	inline mat3 operator -(const mat2& a) const
	{
		return mat3(x - a.x, y - a.y, z);
	}
	inline mat3 operator *(const mat3& a) const
	{
		mat3 tmp = a.transposed();
		
		return mat3(tmp*x, tmp*y, tmp*z);
	}
	inline vec3 operator *(const vec3& a) const
	{
		return vec3(x*a, y*a, z*a);
	}
	inline vec2 operator *(const vec2& a) const
	{
		return vec2(x*a, y*a) / (z*a);
	}
	inline mat3 operator *(const float a) const
	{
		return mat3(x*a, y*a, z*a);
	}
	inline mat3 operator /(const float a) const
	{
		return (*this) * (1.0f / a);
	}
	inline const mat3& operator +=(const mat3& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const mat3& operator +=(const mat2& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const mat3& operator -=(const mat3& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const mat3& operator -=(const mat2& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const mat3& operator *=(const mat3& a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const mat3& operator *=(const float a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const mat3& operator /=(const float a)
	{
		(*this) = (*this) / a; return (*this);
	}
	inline const vec3& operator [](int i) const
	{
		return ((vec3*)this)[i];
	}
	inline vec3& operator [](int i)
	{
		return ((vec3*)this)[i];
	}
	inline const vec3& operator [](size_t i) const
	{
		return ((vec3*)this)[i];
	}
	inline vec3& operator [](size_t i)
	{
		return ((vec3*)this)[i];
	}

	float det(void) const;

	mat3 transposed(void) const;
	mat3 inverse(void) const;
	mat3 inverse2(void) const;

	vec3 extractEulerAngles() const;
    void orthonormalize ();//from ogre

	static inline mat3 Identity()
	{
		return mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	}
	static mat3 RotX(float c, float s);
	static mat3 RotY(float c, float s);
	static mat3 RotZ(float c, float s);
	static mat3 RotAxis(vec3 axis, float angle);
	static mat3 Euler(float yaw, float pitch, float roll);
	static mat3 Scale(vec3 scl);

};

} //namespace Math {

} //namespace Squirrel {
