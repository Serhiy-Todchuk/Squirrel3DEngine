#include "AABB.h"

namespace Squirrel {

namespace Math {

void    AABB::addVertex ( const vec3& v )
{
    if ( v.x < min.x )
        min.x = v.x;

    if ( v.x > max.x )
        max.x = v.x;

    if ( v.y < min.y )
        min.y = v.y;

    if ( v.y > max.y )
        max.y = v.y;

    if ( v.z < min.z )
        min.z = v.z;

    if ( v.z > max.z )
        max.z = v.z;
}

int AABB::classify ( const Plane& pl, int mask ) const
{
    vec3    nearPoint = pl.makeNearPoint ( min, max, mask);

    if ( pl.classify ( nearPoint ) == Plane::IN_FRONT )
        return Plane::IN_FRONT;

    vec3    farPoint = pl.makeFarPoint ( min, max, mask );

    if ( pl.classify ( farPoint ) == Plane::IN_BACK )
        return Plane::IN_BACK;

    return Plane::IN_BOTH;
}

bool	AABB::isEmpty () const
{
	return min.x > max.x || min.y > max.y || min.z > max.z;
}

void    AABB::reset ()
{
    min.x = MAX_COORD;
    min.y = MAX_COORD;
    min.z = MAX_COORD;

    max.x = -MAX_COORD;
    max.y = -MAX_COORD;
    max.z = -MAX_COORD;
}

vec3	AABB::clampPoint(vec3 point) const
{
	point.x = clamp(point.x, min.x, max.x);
	point.y = clamp(point.y, min.y, max.y);
	point.z = clamp(point.z, min.z, max.z);
	return point;
}

vec3    AABB::getVertex ( int index ) const
{
    return vec3 ( index & 1 ? max.x : min.x,
                  index & 2 ? max.y : min.y,
                  index & 4 ? max.z : min.z );
}

vec3	AABB::getCenter () const
{
	return (min + max) * 0.5f;
}

vec3	AABB::getSize () const
{
	return vec3 ( max.x - min.x, max.y - min.y, max.z - min.z );
}

void AABB::merge ( const AABB& box )
{
    if ( box.min.x < min.x )
       min.x = box.min.x;

    if ( box.min.y < min.y )
       min.y = box.min.y;

    if ( box.min.z < min.z )
       min.z = box.min.z;

    if ( box.max.x > max.x )
       max.x = box.max.x;

    if ( box.max.y > max.y )
       max.y = box.max.y;

    if ( box.max.z > max.z )
       max.z = box.max.z;
}

void	AABB::grow ( const vec3& delta )
{
	min -= delta;
	max += delta;
}

void	AABB::grow ( float delta )
{
	min.x -= delta;
	min.y -= delta;
	min.z -= delta;
	max.x += delta;
	max.y += delta;
	max.z += delta;
}
/*
								// distance from point along given direction to this box
float	AABB::getDistanceTo ( const vec3& from, const vec3& dir ) const
{
	return (getVertex ( computeNearPointMask ( dir ) ) - from) & dir;
}

								// estimate distance from point given near point mask and main axis of direction
float	AABB::getDistanceToAlongAxis ( const vec3& from, int index, int axis ) const
{
	return (float)fabs ( (getVertex ( index ) - from) [axis] );
}
*/

void AABB::transform( const mat4& tramsformMatrix)
{
	AABB newBounds;
	newBounds.reset();
	for(int i = 0; i < 8; ++i)
	{
		vec3 vertex = getVertex(i);
		vertex = tramsformMatrix * vertex;
		newBounds.addVertex( vertex );
	}
	min = newBounds.min;
	max = newBounds.max;
}

void	AABB::move  ( const vec3& v )
{
	min += v;
	max += v;
}

void AABB::scale(const vec3& v)
{
	scale( v, vec3(0,0,0) );
}

void AABB::scale(const vec3& v, vec3 center)
{
	max -= center;
	min -= center;
	max = max.mul(v);
	min = min.mul(v);
	max += center;
	min += center;
}

void AABB::rotate(const quat& r)
{
	rotate( r, vec3(0,0,0) );
}

void AABB::rotate(const quat& r, vec3 center)
{
	const unsigned vnum = 8;
	vec3 verts[vnum];
	unsigned i = 0;
	for(i=0; i<vnum; ++i)
		verts[i] = r.apply( getVertex(i)-center );
	reset();
	for(i=0; i<vnum; ++i)
		addVertex( verts[i]+center );
}

bool AABB::getIntersection(vec3 lineStart, vec3 lineEnd, vec3& outNormal, vec3& outPos) const
{
	vec3 point, normal, intersection;
	vec3 dir = (lineEnd - lineStart).normalized();
	 
	Plane planes[6];
	planes[0].set( vec3::AxisX(), -max.x);
	planes[1].set(-vec3::AxisX(),  min.x);
	planes[2].set( vec3::AxisY(), -max.y);
	planes[3].set(-vec3::AxisY(),  min.y);
	planes[4].set( vec3::AxisZ(), -max.z);
	planes[5].set(-vec3::AxisZ(),  min.z);

	for(int i = 0; i < 6; ++i)
	{
		normal	= planes[i].normal();

		//check only planes which are faced to ray
		if((normal * dir) < 0) continue;

		point	= planes[i].point();

		if(getLinePlaneIntersection(lineStart, lineEnd, point, normal, intersection))
		{
			outNormal = normal;
			outPos = intersection;
			return true;
		}
	}

	return false;
}

bool AABB::intersects(const AABB& box) const
{
	if (( max.x < box.min.x) || (min.x > box.max.x) )
		return false;
	if (( max.y < box.min.y) || (min.y > box.max.y) )
		return false;
	if (( max.z < box.min.z) || (min.z > box.max.z) )
		return false;

	return true;
}

bool AABB::intersects(const vec3& pt) const
{
	if((pt.x>max.x)||(pt.x<min.x)) return false;
	if((pt.y>max.y)||(pt.y<min.y)) return false;
	if((pt.z>max.z)||(pt.z<min.z)) return false;
	return true;
}

bool AABB::intersects(const vec3& sphereCenter, float sphereRadius) const
{
	vec3 _max=max + sphereRadius * 2;
	vec3 _min=min - sphereRadius * 2;

	if((sphereCenter.x>_max.x)||(sphereCenter.x<_min.x)) return false;
	if((sphereCenter.y>_max.y)||(sphereCenter.y<_min.y)) return false;
	if((sphereCenter.z>_max.z)||(sphereCenter.z<_min.z)) return false;
	return 1;
}


} //namespace Math {

} //namespace Squirrel {