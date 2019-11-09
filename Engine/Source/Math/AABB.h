#pragma once

#include "vec3.h"
#include "quat.h"
#include "Plane.h"
#include "GeometryTools.h"
#include "macros.h"

#define	MAX_COORD	1e7

namespace Squirrel {

namespace Math {

class SQMATH_API AABB
{
public:
	vec3 max;
	vec3 min;

	inline AABB() { reset(); }
	inline AABB(vec3 max_,vec3 min_) {max=max_;min=min_;}

	//inline void zero() { min.zero(); max.zero(); }

    void    addVertex ( const vec3& v );

	int classify ( const Plane& pl, int mask ) const;

	bool	isEmpty () const;

	void	setMinMax(vec3 max_,vec3 min_) { max = max_; min = min_; }
	void	setPoint(vec3 pt) { max = pt; min = pt; };
	void	setCenterSize(vec3 center, vec3 size) { min = center - size * 0.5f; max = min + size; }

    void    reset ();

    vec3    getVertex ( int index ) const;
	vec3	getCenter () const;
	vec3	getSize () const;

	vec3	clampPoint(vec3 point) const;

    void	merge ( const AABB& box );

	void	grow ( const vec3& delta );
	void	grow ( float delta );

	void transform( const mat4& tramsformMatrix);
	void move(const vec3& v);
	void scale(const vec3& v);
	void scale(const vec3& v, vec3 center);
	void rotate(const quat& r);
	void rotate(const quat& r, vec3 center);

	bool getIntersection(vec3 lineStart, vec3 lineEnd, vec3& outNormal, vec3& outPos) const;

	bool intersects(const AABB& box) const;
	bool intersects(const vec3& pt) const;
	bool intersects(const vec3& sphereCenter, float sphereRadius) const;
};

} //namespace Math {

} //namespace Squirrel {