#pragma once

#include "vec2.h"
#include "macros.h"

namespace Squirrel {

namespace Math {
    
#ifndef _WIN32
# define _finite finite
#endif

// ******************************
// Three-Dimensional Vector Class
// ******************************

class SQMATH_API vec3
{
	public:
	
	float x, y, z;
	
	inline vec3(void) {}

	inline vec3(vec2 a, float _z)
	{
		x = a.x, y = a.y, z = _z;
	}
	inline vec3(float _x, float _y, float _z)
	{
		x = _x, y = _y, z = _z;
	}
	inline vec2 getVec2() const
	{
		return vec2(x, y);
	}
	inline void zero(void)
	{
		x = y = z = 0.0f;
	}
	inline bool isZero(void) const
	{
		return x == 0 && y == 0 && z == 0;
	}
	inline bool isConsiderZero(void) const
	{
		return isFloatConsiderZero(x)&&isFloatConsiderZero(y)&&isFloatConsiderZero(z);
	}
	inline void repair(void)
	{
		if(isFinite()) zero();
	}
	inline bool isFinite(void)
	{
		return ((!_finite(x))||(!_finite(y))||(!_finite(z)));
	}
	inline bool operator ==(const vec3& a) const
	{
		return x == a.x && y == a.y && z == a.z;
	}
	inline bool operator !=(const vec3& a) const
	{
		return x != a.x || y != a.y || z != a.z;
	}
	inline vec3 operator +(const vec3& a) const
	{
		return vec3(x + a.x, y + a.y, z + a.z);
	}
	inline vec3 operator +(const vec2& a) const
	{
		return vec3(x + a.x, y + a.y, z);
	}
	inline vec3 operator -() const
	{
		return vec3( - x, - y, - z);
	}
	inline vec3 operator -(const vec3& a) const
	{
		return vec3(x - a.x, y - a.y, z - a.z);
	}
	inline vec3 operator -(const vec2& a) const
	{
		return vec3(x - a.x, y - a.y, z);
	}
	inline float operator *(const vec3& a) const
	{
		return x*a.x + y*a.y + z*a.z;
	}
	inline float operator *(const vec2& a) const
	{
		return x*a.x + y*a.y + z;
	}
	inline vec3 operator *(const float a) const
	{
		return vec3(x*a, y*a, z*a);
	}
	inline vec3 operator /(const float a) const
	{
		return (*this) * (1.0f / a);
	}
	inline vec3 operator ^(const vec3& a) const
	{
		return vec3(y*a.z - z*a.y, z*a.x - x*a.z, x*a.y - y*a.x);
	}
	inline const vec3& operator +=(const vec3& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const vec3& operator +=(const vec2& a)
	{
		(*this) = (*this) + a; return (*this);
	}
	inline const vec3& operator -=(const vec3& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const vec3& operator -=(const vec2& a)
	{
		(*this) = (*this) - a; return (*this);
	}
	inline const vec3& operator *=(const float a)
	{
		(*this) = (*this) * a; return (*this);
	}
	inline const vec3& operator /=(const float a)
	{
		(*this) = (*this) / a; return (*this);
	}
	inline const vec3& operator ^=(const vec3& a)
	{
		(*this) = (*this) ^ a; return (*this);
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
		return (float)fsqrt(lenSquared());
	}
	inline float lenSquared(void) const
	{
		return (x*x + y*y + z*z);
	}
	inline vec3 mul(vec3 v) const
	{
		return vec3(v.x*this->x,v.y*this->y,v.z*this->z);
	}
	inline vec3 div(vec3 v) const
	{
		return vec3(x/v.x,y/v.y,z/v.z);
	}
	inline vec3 normalized(void) const
	{
		return (*this) / len();
	}
	inline void normalize(void)
	{
		(*this) /= len();
	}
	inline void safeNormalize(void)
	{
		float length = len();
		if(length > 0)
			(*this) /= length;
	}
	inline void operator =(const vec2& a)
	{
		(*this) = vec3(a.x, a.y, 1.0f);
	}

	vec3 mix(const vec3& v, const float f ) const;
	vec3 cosineInterpolate(const vec3& v, const float frac ) const;
	vec3 bicubicInterpolate(const vec3& v, const float frac ) const;
	vec3 reflect(const vec3& normal) const;
	bool isParalel(const vec3& v) const;

	static inline vec3 AxisX() { return vec3(1,0,0); }
	static inline vec3 AxisY() { return vec3(0,1,0); }
	static inline vec3 AxisZ() { return vec3(0,0,1); }
	static inline vec3 Zero()  { return vec3(0,0,0); }
	static inline vec3 One()   { return vec3(1,1,1); }
	static vec3 Random(void);
	static vec3 Polar2Cartesian(const vec3& a);
	static vec3 Cartesian2Polar(const vec3& a);
	static float Mincomp(const vec3& a);
	static float Maxcomp(const vec3& a);
	static vec3 MinComponentwise ( const vec3& v1, const vec3& v2 );
	static vec3 MaxComponentwise ( const vec3& v1, const vec3& v2 );

	//TODO remove these meth0ds
	float getDistanceFromLine(vec3 & project, const vec3 & pointA, const vec3 & pointB);
	float getDistanceFromLineAndProject(vec3 & result, const vec3 & startPoint, const vec3 & dir);
	void removeComponent	(const vec3& V);
	vec3 project(const vec3 & v) const;
	bool getIntersectPlane(vec3 & normal, vec3 & pointOnPlane, vec3 & point1, vec3 & point2);
	float getProjectPlane(vec3 normal, vec3 PointOnPlane, vec3 point);
	float getProjectPlane(vec3 normal, float d, vec3 point);

};

} //namespace Math {

} //namespace Squirrel {

