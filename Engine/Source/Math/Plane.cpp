#include "Plane.h"

namespace Squirrel {

namespace Math {

Plane::Plane(): 
	vec4(0,1,0,0) {}

Plane::Plane(vec3 normal, float minusDistance): 
	vec4(normal, minusDistance) {}

Plane::Plane(vec3 normal, vec3 point): 
	vec4(normal, -(normal * point)) {}

void Plane::set(vec3 normal, float minusDistance)
{
	vec4::set(normal, minusDistance);
}

void Plane::set(vec3 normal, vec3 point)
{
	vec4::set(normal, -(normal * point));
}

void Plane::normalize(void)
{
	(*this) /= normal().len();
}

float Plane::signedDistanceTo ( const vec3& v ) const
{
    return (v * normal()) + w;
}

float Plane::distanceTo ( const vec3& v ) const
{
	return (float)fabsf ( signedDistanceTo ( v ) );
}
                            // get point on plane
vec3 Plane::point () const
{
    return normal() * (-w);
}
                        // classify point
int	Plane::classify ( const vec3& p ) const
{
    float   v = signedDistanceTo ( p );

    if ( v > EPSILON )
        return IN_FRONT;
    else
    if ( v < -EPSILON )
        return IN_BACK;

    return IN_PLANE;
}

bool Plane::intersectByRay ( const vec3& org, const vec3& dir, float& t ) const
{
    float   numer = - (w + (org * normal()));
    float   denom = dir * normal();

    if ( fabsf ( denom ) < EPSILON )
       return false;

	t = numer / denom;

    return true;
}

vec3 Plane::project(vec3 vec) const
{
	float dist = signedDistanceTo(vec);
	return vec + normal() * dist;
}

   // mirror position (point), depends on plane posit.
   ///inline  void    reflectPos ( vec3& v )
   // {
   //     v -= ( ((v * normal()) + w)* 2.0f ) * normal();
   // }

                            // mirror direction, depends only on plane normal
   // inline void    reflectDir ( vec3& v )
   // {
   //     v -= ((v * normal())*2.0f) * normal();
   // }

} //namespace Math {

} //namespace Squirrel {
