#include "vec3.h"

namespace Squirrel {

namespace Math {

vec3 vec3::Random(void)
{
	float x = (float)((rand()*rand()) & 0x7FFF) / 32767.0f;
	float y = (float)((rand()^rand()) & 0x7FFF) / 32767.0f;
	float z = (float)((rand()+rand()) & 0x7FFF) / 32767.0f;
	return vec3(x, y, z);
}

vec3 vec3::Polar2Cartesian(const vec3& a)
{
	return vec3(a.z*fcos(a.x)*fcos(a.y), a.z*fsin(a.x), -a.z*fcos(a.x)*fsin(a.y));
}

vec3 vec3::Cartesian2Polar(const vec3& a)
{
	float tmp = fsqrt(a.x*a.x+a.y*a.y+a.z*a.z);
	float y = acosf(a.x/fsqrt(a.x*a.x+a.z*a.z));
	if(a.z>0)
		y = TWO_PI - y;	
	return vec3(asinf(a.y/tmp), y, tmp);
}

vec3 vec3::mix(const vec3& v, const float f ) const
{
	return (*this)*(1-f)+v*f;
}

vec3 vec3::cosineInterpolate(const vec3& v, const float frac ) const
{
	float ft = frac * PI;
	float f = (1 - fcos(ft)) * 0.5f;
	return (*this)*(1-f)+v*f;
}

vec3 vec3::bicubicInterpolate(const vec3& v, const float frac ) const
{
	float fac1 = 3*powf(1-frac, 2) - 2*powf(1-frac,3);
	float fac2 = 3*powf(frac, 2) - 2*powf(frac, 3);
	return (*this)*fac1+v*fac2;
}

float vec3::Mincomp(const vec3& a)
{
	float ax = fabsf(a.x);
	float ay = fabsf(a.y);
	float az = fabsf(a.z);
	if(ax < ay && ax < az) return ax;
	if(ay < az) return ay;
	return az;
}

float vec3::Maxcomp(const vec3& a)
{
	float ax = fabsf(a.x);
	float ay = fabsf(a.y);
	float az = fabsf(a.z);
	if(ax > ay && ax > az) return ax;
	if(ay > az) return ay;
	return az;
}

vec3 vec3::MinComponentwise ( const vec3& v1, const vec3& v2 )
{
	return vec3 ( v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y, v1.z < v2.z ? v1.z : v2.z );
}

vec3 vec3::MaxComponentwise ( const vec3& v1, const vec3& v2 )
{
	return vec3 ( v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y, v1.z > v2.z ? v1.z : v2.z );
}

float vec3::getDistanceFromLine(vec3 & project, const vec3 & pointA, const vec3 & pointB)
{
	vec3 ba = pointB - pointA;
	float leng = ba.len();
	if (isFloatConsiderZero(leng))	ba.zero();
	else							ba *= 1.0f / leng;
	vec3 pa = (*this) - pointA;
	float k = pa*ba;
	project = pointA + ba * k;//proektsiya (this) na liniyu AB
	vec3 diff = (*this) - project;
	return diff.len();
}

float vec3::getDistanceFromLineAndProject(vec3 & result, const vec3 & startPoint, const vec3 & dir)
{
	vec3 pa = (*this) - startPoint;
	float k = pa*dir;
	result = startPoint + dir * k;
	vec3 diff = (*this) - result;
	return diff.len();
}

void vec3::removeComponent	(const vec3& V)
{
	float dot = (*this)*V;
	(*this) = (*this) - V * dot;
}

vec3 vec3::project(const vec3 & v) const
{
	float dot = (*this)*v;
	return (v * dot);
}

bool vec3::getIntersectPlane(vec3 & normal, vec3 & pointOnPlane, vec3 & point1, vec3 & point2)
{
	vec3 diff = point2 - point1;
	float d2 = normal*diff;
	if (isFloatConsiderZero(d2))	return false;
	float d1 = normal*(pointOnPlane - point1);
	float u = d1 / d2;
	*this = point1 + diff * u;
	return true;
}

float vec3::getProjectPlane(vec3 normal, vec3 PointOnPlane, vec3 point)
{
	float plane_d = -(normal*PointOnPlane);
	float dist=normal*point+plane_d;
	(*this)=point-(normal*dist);
	return dist;
}

float vec3::getProjectPlane(vec3 normal, float d, vec3 point)
{
	float dist=normal*point+d;
	(*this)=point-(normal*dist);
	return dist;
}

vec3 vec3::reflect( const vec3& normal) const
{
	return (*this)-(normal*2*(normal*(*this)));
}

bool vec3::isParalel(const vec3& v) const
{
	return (((x/v.x)==(y/v.y))&&((x/v.x)==(z/v.z)));
}

} //namespace Math {

} //namespace Squirrel {

