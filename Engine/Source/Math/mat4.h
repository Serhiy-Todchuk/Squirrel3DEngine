#pragma once

#include "BasicUtils.h"
#include "vec4.h"
#include "mat3.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

// *****************************
// Four-Dimensional Matrix Class
// *****************************

class SQMATH_API mat4
{
	public:

	vec4 x, y, z, w;

	inline mat4(void) {}
	
	inline mat4(const float * arr)
	{
		fromArray(arr);
	}
	inline mat4(float m11, float m12, float m13, float m14,
	            float m21, float m22, float m23, float m24,
	            float m31, float m32, float m33, float m34,
	            float m41, float m42, float m43, float m44)
	{
		x = vec4(m11, m12, m13, m14);
		y = vec4(m21, m22, m23, m24);
		z = vec4(m31, m32, m33, m34);
		w = vec4(m41, m42, m43, m44);
	}
	inline mat4(const vec4& mx, const vec4& my, const vec4& mz, const vec4 &mw)
	{
		x = mx, y = my, z = mz, w = mw;
	}
	inline mat4 identity(void)
	{
		x = vec4(1.0f, 0.0f, 0.0f, 0.0f);
		y = vec4(0.0f, 1.0f, 0.0f, 0.0f);
		z = vec4(0.0f, 0.0f, 1.0f, 0.0f);
		w = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		return *this;
	}
	inline void zero(void)
	{
		x.zero();
		y.zero();
		z.zero();
		w.zero();
	}
	inline void fromArray(const float * arr)
	{
		x = vec4(&arr[0]), y = vec4(&arr[4]), z = vec4(&arr[8]), w = vec4(&arr[12]);
	}

	inline const bool operator ==(const mat4& a) const
	{
		return x == a.x && y == a.y && z == a.z && w == a.w;
	}
	inline const bool operator !=(const mat4& a) const
	{
		return x != a.x || y != a.y || z != a.z || w != a.w;
	}
	inline mat4 operator +(const mat4& a) const
	{
		return mat4(x + a.x, y + a.y, z + a.z, w + a.w);
	}
	inline mat4 operator +(const mat3& a) const
	{
		return mat4(x + a.x, y + a.y, z + a.z, w);
	}
	inline mat4 operator -(const mat4& a) const
	{
		return mat4(x - a.x, y - a.y, z - a.z, w - a.w);
	}
	inline mat4 operator -(const mat3& a) const
	{
		return mat4(x - a.x, y - a.y, z - a.z, w);
	}
	inline mat4 operator *(const mat4& a) const
	{
		mat4 tmp = a.transposed();
		return mat4(tmp*x, tmp*y, tmp*z, tmp*w);
	}
	inline vec4 operator *(const vec4& a) const
	{
		return vec4(x*a, y*a, z*a, w*a);
	}
	inline vec3 operator *(const vec3& a) const
	{
		return vec3(x*a, y*a, z*a) / (w*a);
	}
	inline mat4 operator *(const float a) const
	{
		return mat4(x*a, y*a, z*a, w*a);
	}
	inline mat4 operator /(const float a) const
	{
		return (*this) * (1.0f / a);
	}
	inline const mat4& operator +=(const mat4& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const mat4& operator +=(const mat3& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const mat4& operator -=(const mat4& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const mat4& operator -=(const mat3& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const mat4& operator *=(const mat4& a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const mat4& operator *=(const float a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const mat4& operator /=(const float a)
	{
		(*this) = (*this) / a; return (*this);
	}
	inline const vec4& operator [](int i) const
	{
		return ((vec4*)this)[i];
	}
	inline vec4& operator [](int i)
	{
		return ((vec4*)this)[i];
	}
	inline const vec4& operator [](size_t i) const
	{
		return ((vec4*)this)[i];
	}
	inline vec4& operator [](size_t i)
	{
		return ((vec4*)this)[i];
	}

	inline mat3 getMat3(void) const
	{
		return mat3(x.x, x.y, x.z,
		            y.x, y.y, y.z,
		            z.x, z.y, z.z);
	}

	inline void setMat3(mat3 m)
	{
		x.x = m.x.x;  x.y = m.x.y;  x.z = m.x.z;
		y.x = m.y.x;  y.y = m.y.y;  y.z = m.y.z;
		z.x = m.z.x;  z.y = m.z.y;  z.z = m.z.z;
	}

	inline vec3 getTranslate(void) const
	{
		return vec3(x.w, y.w, z.w);
	}

	inline void setTranslate(vec3 v) 
	{
		x.w = v.x;
		y.w = v.y;
		z.w = v.z;
	}

    float minor( const size_t r0, const size_t r1, const size_t r2,
						const size_t c0, const size_t c1, const size_t c2 ) const;

    mat4	adjoint()			const;
    float	determinant()		const;
    mat4	inverse()			const;
	mat4	transposed(void)	const;
	vec3	extractScale()		const;

	static inline mat4 Identity()
	{
		return mat4( 1, 0, 0, 0,
					 0, 1, 0, 0,
					 0, 0, 1, 0,
					 0, 0, 0, 1 );
	}
	static mat4 RotX(float c, float s);
	static mat4 RotY(float c, float s);
	static mat4 RotZ(float c, float s);
	static mat4 RotAxis(vec3 axis, float angle);
	static mat4 Euler(float yaw, float pitch, float roll);
	static mat4 Scale(vec3 scl);
	static mat4 Translate(vec3 pos);
	static mat4 Transform(vec3 pos, mat3 rot, vec3 scl);
	static mat4 Transform(vec3 pos, vec3 scl);


};

} //namespace Math {

} //namespace Squirrel {
