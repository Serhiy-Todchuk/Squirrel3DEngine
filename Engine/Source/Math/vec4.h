#pragma once

#include "vec3.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

// *****************************
// Four-Dimensional Vector Class
// *****************************

class SQMATH_API vec4
{
	public:
	
	float x, y, z, w;
	
	inline vec4(void) {}
	
	inline vec4(const float * arr)
	{
		x = arr[0], y = arr[1], z = arr[2], w = arr[3];
	}
	inline vec4(float _x, float _y, float _z, float _w)
	{
		x = _x, y = _y, z = _z, w = _w;
	}
	inline vec4(vec3 _v, float _w)
	{
		x = _v.x, y = _v.y, z = _v.z, w = _w;
	}
	inline void zero(void)
	{
		x = y = z = w = 0.0f;
	}
	inline bool operator ==(const vec4& a) const
	{
		return x == a.x && y == a.y && z == a.z && w == a.w;
	}
	inline bool operator !=(const vec4& a) const
	{
		return x != a.x || y != a.y || z != a.z || w != a.w;
	}
	inline vec4 operator +(const vec4& a) const
	{
		return vec4(x + a.x, y + a.y, z + a.z, w + a.w);  
	}
	inline vec4 operator +(const vec3& a) const
	{
		return vec4(x + a.x, y + a.y, z + a.z, w);
	}
	inline vec4 operator -(const vec4& a) const
	{
		return vec4(x - a.x, y - a.y, z - a.z, w - a.w);
	}
	inline vec4 operator -(const vec3& a) const
	{
		return vec4(x - a.x, y - a.y, z - a.z, w);
	}
	inline float operator *(const vec4& a) const
	{
		return x*a.x + y*a.y + z*a.z + w*a.w;
	}
	inline float operator *(const vec3& a) const
	{
		return x*a.x + y*a.y + z*a.z + w;
	}
	inline vec4 operator *(const float a) const
	{
		return vec4(x*a, y*a, z*a, w*a);
	}
	inline vec4 operator /(const float a) const
	{
		return (*this) * (1.0f / a);
	}
	inline const vec4& operator +=(const vec4& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const vec4& operator +=(const vec3& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const vec4& operator -=(const vec4& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const vec4& operator -=(const vec3& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const vec4& operator *=(const float a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const vec4& operator /=(const float a)
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
	inline float operator [](size_t i) const
	{
		return ((float*)this)[i];
	}
	inline float& operator [](size_t i)
	{
		return ((float*)this)[i];
	}
	inline float len(void) const
	{
		return (float)fsqrt( lenSquared() );
	}
	inline float lenSquared(void) const
	{
		return x*x + y*y + z*z + w*w;
	}
	inline void operator =(const vec3& a)
	{
		(*this) = vec4(a.x, a.y, a.z, 1.0f);
	}
	inline vec3 getVec3(void) const
	{
		return vec3(x,y,z);
	}
	inline void set(vec3 xyz,float _w)
	{
		(*this)=vec4(xyz.x,xyz.y,xyz.z,_w);
	}

	vec4 normalized(void) const;
	void normalize(void);
	void mix(const vec4& v1, const vec4& v2, const float f );

	static inline vec4 Zero() { return vec4(0, 0, 0, 0); }

};

} //namespace Math {

} //namespace Squirrel {
