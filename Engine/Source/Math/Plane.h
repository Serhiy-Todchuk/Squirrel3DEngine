// **************************************************************************
// Crazy Dreamer, 2010
// **************************************************************************

#pragma once

#include "vec4.h"
#include "macros.h"

namespace Squirrel {

namespace Math {

class SQMATH_API Plane : public vec4
{
public:
	enum
	{
		IN_FRONT = 1,
		IN_BACK  = 2,
		IN_BOTH  = 3,
		IN_PLANE = 4
	};

public:
	Plane();
	Plane(vec3 normal, float minusDistance);
	Plane(vec3 normal, vec3 point);

	void set(vec3 normal, float minusDistance);
	void set(vec3 normal, vec3 point);

	void normalize(void);

    float signedDistanceTo ( const vec3& v ) const;

	float	distanceTo ( const vec3& v ) const;
	
	// get point on plane
    vec3    point () const;
    
	// classify point
    int	classify ( const vec3& p ) const;

    bool intersectByRay ( const vec3& org, const vec3& dir, float& t ) const;

	vec3 project(vec3 vec) const;

	inline vec3 normal(void) const
	{
		return vec3(x, y, z);
	}

	inline float distance(void) const
	{
		return w;
	}

    inline vec3    makeNearPoint ( const vec3& minPoint, const vec3& maxPoint, int nearPointMask ) const
    {
        return vec3 ( nearPointMask & 1 ? maxPoint.x : minPoint.x,
                      nearPointMask & 2 ? maxPoint.y : minPoint.y,
                      nearPointMask & 4 ? maxPoint.z : minPoint.z );
    }

    inline vec3    makeFarPoint ( const vec3& minPoint, const vec3& maxPoint, int nearPointMask ) const
    {
        return vec3 ( nearPointMask & 1 ? minPoint.x : maxPoint.x,
                      nearPointMask & 2 ? minPoint.y : maxPoint.y,
                      nearPointMask & 4 ? minPoint.z : maxPoint.z );
    }
};

} //namespace Math {

} //namespace Squirrel {
