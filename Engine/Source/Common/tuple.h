#pragma once

#include <stdlib.h>
#include "types.h"

namespace Squirrel {

template <class T>
class tuple2
{
public:
	T x, y;
	
	inline tuple2(void) {}
	
	inline bool isZero() const 
	{ 
		return x == 0 && y == 0; 
	}
	inline tuple2(T _x, T _y)
	{
		x = _x, y = _y;
	}
	inline void zero(void)
	{
		x = y = 0;
	}
	inline bool operator <(const tuple2& a) const
	{
		if ( x < a.x )	return true;
		if ( x > a.x )	return false;
		if ( y < a.y )	return true;
		if ( y > a.y )	return false;
		return false;
	}
	inline bool operator ==(const tuple2& a) const
	{
		return x == a.x && y == a.y;
	}
	inline bool operator !=(const tuple2& a) const
	{
		return x != a.x || y != a.y;
	}
	inline T operator [](T i) const
	{
		return ((T*)this)[i];
	}
	inline T& operator [](T i)
	{
		return ((T*)this)[i];
	}
};

template <class T>
class tuple3
{
public:
	T x, y, z;
	
	inline tuple3(void) {}
	
	inline tuple3(T _x, T _y, T _z)
	{
		x = _x, y = _y, z = _z;
	}
	inline void zero(void)
	{
		x = y = z = 0;
	}
	inline bool operator <(const tuple3& a) const
	{
		if ( x < a.x )	return true;
		if ( x > a.x )	return false;
		if ( y < a.y )	return true;
		if ( y > a.y )	return false;
		if ( z < a.z )	return true;
		if ( z > a.z )	return false;
		return false;
	}
	inline bool operator ==(const tuple3& a) const
	{
		return x == a.x && y == a.y && z == a.z;
	}
	inline bool operator !=(const tuple3& a) const
	{
		return x != a.x || y != a.y || z != a.z;
	}
	inline T operator [](T i) const
	{
		return ((T*)this)[i];
	}
	inline T& operator [](T i)
	{
		return ((T*)this)[i];
	}
	
};

template <class T>
class tuple4
{
public:
	T x, y, z, w;
	
	inline tuple4(void) {}
	
	inline tuple4(T _x, T _y, T _z, T _w)
	{
		x = _x, y = _y, z = _z, w = _w;
	}
	inline void zero(void)
	{
		x = y = z = w = 0;
	}
	inline bool operator <(const tuple4& a) const
	{
		if ( x < a.x )	return true;
		if ( x > a.x )	return false;
		if ( y < a.y )	return true;
		if ( y > a.y )	return false;
		if ( z < a.z )	return true;
		if ( z > a.z )	return false;
		if ( w < a.w )	return true;
		if ( w > a.w )	return false;
		return false;
	}
	inline bool operator ==(const tuple4& a) const
	{
		return x == a.x && y == a.y && z == a.z && w == a.w;
	}
	inline bool operator !=(const tuple4& a) const
	{
		return x != a.x || y != a.y || z != a.z || w != a.w;
	}
	inline T operator [](T i) const
	{
		return ((T*)this)[i];
	}
	inline T& operator [](T i)
	{
		return ((T*)this)[i];
	}
};

class tuple2i: public tuple2<int>
{
public:
	
	tuple2i(void) {}
	tuple2i(int _x, int _y)
	{
		x = _x, y = _y;
	}
	inline void operator +=(const tuple2i& a) 
	{
		x += a.x; y += a.y;
	}
	inline void operator -=(const tuple2i& a) 
	{
		x -= a.x; y -= a.y;
	}
	inline tuple2i operator +(const tuple2i& a) const
	{
		return tuple2i(x + a.x, y + a.y);
	}
	inline tuple2i operator -(const tuple2i& a) const
	{
		return tuple2i(x - a.x, y - a.y);
	}

	inline void operator +=(int a) 
	{
		x += a; y += a;
	}
	inline void operator -=(int a) 
	{
		x -= a; y -= a;
	}
	inline tuple2i operator +(int a) const
	{
		return tuple2i(x + a, y + a);
	}
	inline tuple2i operator -(int a) const
	{
		return tuple2i(x - a, y - a);
	}

	inline tuple2i operator *(int a) const
	{
		return tuple2i(x * a, y * a);
	}
	inline tuple2i operator /(int a) const
	{
		return tuple2i(x / a, y / a);
	}
};

class tuple3i: public tuple3<int>
{
public:
	
	tuple3i(void) {}
	tuple3i(int _x, int _y, int _z)
	{
		x = _x, y = _y, z = _z;
	}
	tuple3i(const tuple2i& a, int _z)
	{
		x = a.x, y = a.y, z = _z;
	}
	
	inline tuple3i operator -(int a) const
	{
		return tuple3i(x - a, y - a, z - a);
	}
	inline tuple3i operator -(const tuple3i& a) const
	{
		return tuple3i(x - a.x, y - a.y, z - a.z);
	}
	inline tuple3i operator /(int a) const
	{
		return tuple3i(x / a, y / a, z / a);
	}	
	
	static tuple3i Random(void)
	{
		int x = (int)((rand()*rand()) & 0x7FFF) / 32767;
		int y = (int)((rand()^rand()) & 0x7FFF) / 32767;
		int z = (int)((rand()+rand()) & 0x7FFF) / 32767;
		return tuple3i(x, y, z);
	}
};

class tuple4i: public tuple4<int>
{
public:
	
	tuple4i(void) {}
	tuple4i(int _x, int _y, int _z, int _w)
	{
		x = _x, y = _y, z = _z, w = _w;
	}
};

class tuple2s: public tuple2<short>
{
public:
	
	tuple2s(void) {}
	tuple2s(short _x, short _y)
	{
		x = _x, y = _y;
	}
};

class tuple3b: public tuple3<int8>
{
	public:
	
	tuple3b(void) {}
	tuple3b(int8 _x, int8 _y, int8 _z)
	{
		x = _x, y = _y, z = _z;
	}

	inline void setNormalized(float value, int index)
	{
		(*this)[index] = (int8)(value * 127.0f);
	}
	inline float getNormalized(int index)
	{
		return float((*this)[index]) / 127.0f;
	}
};

class tuple4b: public tuple4<int8>
{
public:
	
	tuple4b(void) {}
	inline tuple4b(int8 _x, int8 _y, int8 _z, int8 _w)
	{
		x = _x, y = _y, z = _z, w = _w;
	}
	inline tuple4b(const tuple3b& a)
	{
		x = a.x, y = a.y, z = a.z, w = 0;
	}
	inline tuple3b get3b() const
	{
		return tuple3b(x,y,z);
	}
	inline void setNormalized(float value, int index)
	{
		(*this)[index] = (int8)(value * 127.0f);
	}
	inline float getNormalized(int index)
	{
		return float((*this)[index]) / 127.0f;
	}
};

class tuple4ub: public tuple4<uint8>
{
public:
	
	tuple4ub(void) {}
	inline tuple4ub(uint8 _x, uint8 _y, uint8 _z, uint8 _w)
	{
		x = _x, y = _y, z = _z, w = _w;
	}
};

//extend math to have GLSL-like integer types
namespace Math {

typedef tuple2i ivec2;
typedef tuple3i ivec3;
typedef tuple4i ivec4;
	
} //namespace Math {

} //namespace Squirrel {