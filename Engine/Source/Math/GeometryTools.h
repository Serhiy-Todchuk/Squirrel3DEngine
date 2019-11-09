// GeometryTools.h: interface for the GeometryTools class.

#pragma once

#include "vec3.h"
#include "macros.h"

#define BEHIND		0
#define INTERSECTS	1
#define FRONT		2

namespace Squirrel {

namespace Math {

SQMATH_API float __cdecl triangleArea ( const vec3& a, const vec3& b, const vec3& c );

SQMATH_API bool __cdecl areTrianglesIntersect(	vec3 tri1v0, vec3 tri1v1, vec3 tri1v2, 
							vec3 tri2v0, vec3 tri2v1, vec3 tri2v2);
SQMATH_API bool __cdecl areTrianglesIntersect2(	vec3 t1v0, vec3 t1v1, vec3 t1v2, 
							vec3 t2v0, vec3 t2v1, vec3 t2v2, vec3 &point, vec3 &normal, vec3 &offset);
SQMATH_API bool __cdecl getLnTriIntersect(vec3 p1, vec3 p2, vec3 v1, vec3 v2, vec3 v3, vec3 n, vec3 &res);
SQMATH_API bool __cdecl getLinePlaneIntersection( vec3 lbegin, vec3 lend,
				               vec3 PointInPlane, vec3 PlaneNormal,
				               vec3 &intersection );
SQMATH_API bool __cdecl isPointInsideTriangle( vec3 intersection, vec3 v0, vec3 v1, vec3 v2);
SQMATH_API bool __cdecl isPointInsideTriangleV(vec3 intersection, vec3 v0, vec3 v1, vec3 v2);
SQMATH_API bool __cdecl isPointInsideTriangleFast(vec3 intersection, vec3 v0, vec3 v1, vec3 v2,vec3 n);

//	This returns the normal of a polygon (The direction the polygon is facing)
SQMATH_API vec3 __cdecl getNormalToTriangle( vec3 v0, vec3 v1, vec3 v2);
SQMATH_API vec3 __cdecl getNormalToTriangle( vec3 poly[] );

// This returns the point on the line segment vA_vB that is closest to point vPoint
SQMATH_API vec3 __cdecl closestPointOnLine(vec3 vA, vec3 vB, vec3 point);

// Orthogonalization of vectors with Gram-Schmidt method;
// returns v2 orthogonalized to v1
SQMATH_API vec3 __cdecl orthogonalize( const vec3& v1, const vec3& v2 );

//calculates tangent basis of triangle from 3 vertex positions and vertex texture coordinates
SQMATH_API void __cdecl calcTriangleTangentBasis(	const vec3& E, const vec3& F, const vec3& G,
													const vec2& texE, const vec2& texF, const vec2& texG,
													vec3& tangentX, vec3& tangentY );

// This returns the distance the plane is from the origin (0, 0, 0)
// It takes the normal to the plane, along with ANY point that lies on the plane (any corner)
SQMATH_API float __cdecl planeDistance(vec3 normal, vec3 point);

// This takes a triangle (plane) and line and returns true if they intersected
SQMATH_API bool __cdecl intersectedPlane(vec3 poly[], vec3 line[], vec3 &normal, float &originDistance);

// This returns the angle between 2 vectors
SQMATH_API float __cdecl angleBetweenVectors(vec3 Vector1, vec3 Vector2);

// This returns an intersection point of a polygon and a line (assuming intersects the plane)
SQMATH_API vec3 __cdecl intersectionPoint(vec3 normal, vec3 line[], double distance);

// This returns true if the intersection point is inside of the polygon
SQMATH_API bool __cdecl insidePolygon(vec3 intersection, vec3 poly[], long VerticeCount);

// Use this function to test collision between a line and polygon
SQMATH_API bool __cdecl intersectedPolygon(vec3 poly[], vec3 line[], int VerticeCount);

// This function classifies a sphere according to a plane. (BEHIND, in FRONT, or INTERSECTS)
SQMATH_API int __cdecl classifySphere(vec3 &center, 
				   vec3 &normal, vec3 &point, float radius, float &distance);

// This takes in the sphere center, radius, polygon vertices and vertex count.
// This function is only called if the intersection point failed.  The sphere
// could still possibly be intersecting the polygon, but on it's edges.
SQMATH_API bool __cdecl edgeSphereCollision(vec3 center, 
						 vec3 poly[], int VertexCount, float radius, vec3 &point, float &distance);

// This returns true if the sphere is intersecting with the polygon.
SQMATH_API bool __cdecl spherePolygonCollision(vec3 poly[], 
							vec3 &center, int VertexCount, float radius);
// This returns true if the sphere is intersecting with the polygon.
SQMATH_API bool __cdecl sphereTriangleCollision(vec3 v1, vec3 v2, vec3 v3, 
							 vec3 center, float radius);
SQMATH_API bool __cdecl sphereTriangleCollision2(vec3 v1, vec3 v2, vec3 v3, 
							  vec3 center, float radius, 
							  vec3 &point, vec3 &normal, vec3 &offset);

// This returns the offset the sphere needs to move in order to not intersect the plane
SQMATH_API vec3 __cdecl getCollisionOffset(vec3 &normal, float radius, float distance);

// if there is intersection returns the distance along ray to the first intersecting point ,
// otherwise returns negative value (no intersection)
SQMATH_API float __cdecl raySphereIntersection(vec3 rO, vec3 rV, vec3 sO, float sR);


} //namespace Math {

} //namespace Squirrel {