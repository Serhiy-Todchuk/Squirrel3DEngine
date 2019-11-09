#include "GeometryTools.h"

#ifndef _WIN32
#	define _isnan isnan
#endif

namespace Squirrel {

namespace Math {

float triangleArea ( const vec3& a, const vec3& b, const vec3& c )
{
    vec3 v1 = b - a;
    vec3 v2 = c - a;
    return ((v1.y*v2.z + v1.z*v2.x + v1.x*v2.y) - (v1.y*v2.x + v1.x*v2.z + v1.z*v2.y));
}

bool getLnTriIntersect(vec3 p1, vec3 p2, vec3 v1, vec3 v2, vec3 v3, vec3 n, vec3 &res)
{
	// ‚˚˜ËÒÎˇÂÏ ‡ÒÒÚÓˇÌËˇ ÏÂÊ‰Û ÍÓÌˆ‡ÏË ÓÚÂÁÍ‡ Ë ÔÎÓÒÍÓÒÚ¸˛ ÚÂÛ„ÓÎ¸ÌËÍ‡.
	float r1 = n * (p1 - v1);
	float r2 = n * (p2 - v1);
	// ÂÒÎË Ó·‡ ÍÓÌˆ‡ ÓÚÂÁÍ‡ ÎÂÊ‡Ú ÔÓ Ó‰ÌÛ ÒÚÓÓÌÛ ÓÚ ÔÎÓÒÍÓÒÚË, ÚÓ ÓÚÂÁÓÍ
	// ÌÂ ÔÂÂÒÂÍ‡ÂÚ ÚÂÛ„ÓÎ¸ÌËÍ.
	if( sign(r1) == sign(r2) ) return false;
	// ‚˚˜ËÒÎˇÂÏ ÚÓ˜ÍÛ ÔÂÂÒÂ˜ÂÌËˇ ÓÚÂÁÍ‡ Ò ÔÎÓÒÍÓÒÚ¸˛ ÚÂÛ„ÓÎ¸ÌËÍ‡.
	res = (p1 + ((p2 - p1) * (-r1 / (r2 - r1))));
	// ÔÓ‚ÂˇÂÏ, Ì‡ıÓ‰ËÚÒˇ ÎË ÚÓ˜Í‡ ÔÂÂÒÂ˜ÂÌËˇ ‚ÌÛÚË ÚÂÛ„ÓÎ¸ÌËÍ‡.
	if( (((v2 - v1)^(res - v1)) * n) <= 0) return false;
	if( (((v3 - v2)^(res - v2)) * n) <= 0) return false;
	if( (((v1 - v3)^(res - v3)) * n) <= 0) return false;
	return true; 
}

//-----------------------------------------------------------------------------
// Name: areTrianglesIntersect()
// Desc: Determine whether triangle "tri1" intersects "tri2".
//-----------------------------------------------------------------------------
bool areTrianglesIntersect(	vec3 tri1v0, vec3 tri1v1, vec3 tri1v2, 
							vec3 tri2v0, vec3 tri2v1, vec3 tri2v2)
{
	bool bIntersect = false;
	vec3 point;
	// Create a normal for 'tri1'
	vec3 EdgeVec1 = tri1v1 - tri1v0;
	vec3 EdgeVec2 = tri1v2 - tri1v0;
	vec3 tri1n = EdgeVec1 ^ EdgeVec2;
	//tri1n.normalize(); // Some people feel compelled to normalize this, but it's not really necessary.
	// Check the first line segment of triangle #2 against triangle #1
    // The first line segment is defined by vertices v0 and v1.
	bIntersect = getLinePlaneIntersection( tri2v0,      // Line start
		                                   tri2v1,      // Line end
				                           tri1v0,      // A point in the plane
							               tri1n,		// The plane's normal
				                           point );     // Holds the intersection point, if the function returns true
	if( bIntersect == true )
	{
		// The line segment intersects the plane, but does it actually go 
		// through the triangle?
		if( isPointInsideTriangle( point, tri1v0, tri1v1, tri1v2 ) )	return true;
	}
	// Check the second line segment of triangle #2 against triangle #1
    // The second line segment is defined by vertices v1 and v2.
	bIntersect = getLinePlaneIntersection( tri2v1,      // Line start
		                                   tri2v2,      // Line end
				                           tri1v0,      // A point in the plane
							               tri1n,		// The plane's normal
				                           point );     // Holds the intersection point, if the function returns true
	if( bIntersect == true )
	{
		// The line segment intersects the plane, but does it actually go 
		// through the triangle?
		if( isPointInsideTriangle( point, tri1v0, tri1v1, tri1v2 ) )	return true;
	}
	// Check the third line segment of triangle #2 against triangle #1
    // The third line segment is defined by vertices v2 and v0.
	bIntersect = getLinePlaneIntersection( tri2v2,      // Line start
		                                   tri2v0,      // Line end
				                           tri1v0,      // A point in the plane
							               tri1n,		// The plane's normal
				                           point );     // Holds the intersection point, if the function returns true
	if( bIntersect == true )
	{
		// The line segment intersects the plane, but does it actually go 
		// through the triangle?
		if( isPointInsideTriangle( point, tri1v0, tri1v1, tri1v2 ) )	return true;
	}

    return false;
}

//-----------------------------------------------------------------------------
// Name: areTrianglesIntersect()
// Desc: Determine whether triangle "tri1" intersects "tri2".
//-----------------------------------------------------------------------------
bool areTrianglesIntersect2(	vec3 t1v0, vec3 t1v1, vec3 t1v2, 
							vec3 t2v0, vec3 t2v1, vec3 t2v2, vec3 &point, vec3 &normal, vec3 &offset)
{
	//bool bIntersect = false;
	// Create a normal for 'tri1'
	//vec3 t1n = (t1v1 - t1v0) ^ (t1v2 - t1v0);
	//tri1n.normalize(); // Some people feel compelled to normalize this, but it's not really necessary.
	normal=getNormalToTriangle(t1v0,t1v1,t1v2);
	if( getLinePlaneIntersection( t2v0,t2v1,t1v0,normal,point ) )
	{
		if( isPointInsideTriangle( point, t1v0, t1v1, t1v2 ) )
			return true;
	}
	if( getLinePlaneIntersection( t2v1,t2v2,t1v0,normal,point ) )
	{
		if( isPointInsideTriangle( point, t1v0, t1v1, t1v2 ) )
			return true;
	}
	if( getLinePlaneIntersection( t2v2,t2v0,t1v0,normal,point ) )
	{
		if( isPointInsideTriangle( point, t1v0, t1v1, t1v2 ) )
			return true;
	}
    return false;
}

//-----------------------------------------------------------------------------
// Name : getLinePlaneIntersectionPoint
// Desc : Determine whether a line or ray defined by "vLineStart" and "vLineEnd",
//        intersects with a plane which is defined by "vPlaneNormal" and
//        "vPointInPlane". If it doesn't, return false, otherwise, return true
//        and set "vIntersection" to the intersection point in 3D space.
//-----------------------------------------------------------------------------
bool getLinePlaneIntersection( vec3 lbegin, vec3 lend,
				               vec3 PointInPlane, vec3 PlaneNormal,
				               vec3 &intersection )
{
	vec3 dir = lend - lbegin;
	vec3 L1  = PointInPlane - lbegin;
	float	 fLineLength = dir * PlaneNormal;
    float    fDistanceFromPlane = L1 * PlaneNormal;
	// How far from Linestart , intersection is as a percentage of 0 to 1
	float    percentage = fDistanceFromPlane / fLineLength;
	// Check the line's length allowing for some tolerance for floating point
	// rounding errors. If it's 0 or really close to 0, the line is parallel to
	// the plane and can not intersect it.
	if( fabsf( fLineLength ) < 0.001f )
        return false;
	if( percentage < 0.0f || // The plane is behind the start of the line
		percentage > 1.0f )  // The line segment does not reach the plane
        return false;
	// Add the percentage of the line to line start
	intersection = lbegin + dir * percentage;
	return true;
}

//-----------------------------------------------------------------------------
// Name : isPointInsideTriangle
// Desc : Determine wether a point in 3D space, "vIntersectionPoint", can be
//        considered to be inside of the three vertices of a triangle as
//        defined by "v".
//-----------------------------------------------------------------------------
bool isPointInsideTriangle( vec3 intersection, vec3 v0, vec3 v1, vec3 v2)
{
	vec3 vecs[3];
	float angle=0;
	float TotalAngle = 0.0f; // As radians
	// Create and normalize three vectors that radiate out from the
	// intersection point towards the triangle's three vertices.
	vecs[0] = intersection - v0;
	vecs[0].normalize();
	vecs[1] = intersection - v1;
	vecs[1].normalize();
	vecs[2] = intersection - v2;
	vecs[2].normalize();
	// We then sum together the angles that exist between each of the vectors.
	//
	// Here's how:
	// 1. Use dotProduct() to get cosine of the angle between the two vectors.
	// 2. Use acos() to convert cosine back into an angle.
	// 3. Add angle to fTotalAngle to keep track of running sum.

	//*
	angle = acosf( vecs[0] * vecs[1] );
	//if(!IsValidFloat(angle)) return false;
	TotalAngle  = angle;
	angle = acosf( vecs[1] * vecs[2] );
	//if(!IsValidFloat(angle)) return false; 
	TotalAngle += angle;
	angle = acosf( vecs[2] * vecs[0] );
	//if(!IsValidFloat(angle)) return false;
	TotalAngle += angle;
	//*/

	//TotalAngle-=((vecs[0] * vecs[1]) - 1);
	//TotalAngle-=((vecs[1] * vecs[2]) - 1);
	//TotalAngle-=((vecs[2] * vecs[0]) - 1);
	//Log::ShowWMsg("angle",TotalAngle);

	/*
	TotalAngle  = ACOS( vecs[0] * vecs[1] );
	TotalAngle += ACOS( vecs[1] * vecs[2] );
	TotalAngle += ACOS( vecs[2] * vecs[0] );
	*/
	// If we are able to sum together all three anlges and get 360.0, the
	// intersection point is inside the triangle.
	//
	// We can check this by taking away 6.28 radians (360 degrees) away from
	// fTotalAngle and if we're left with 0 (allowing for some tolerance) the
	// intersection point is definitely inside the triangle.
	if( fabsf( TotalAngle - TWO_PI ) < 0.01f )	return true;
	//if( IsFloatConsiderZero( TotalAngle - TWO_PI ) )	return true;
	//if( fabsf( TotalAngle - 4.0f ) < 0.1f )	return true;

	return false;
}

//-----------------------------------------------------------------------------
// Name : isPointInsideTriangleV
//-----------------------------------------------------------------------------
bool isPointInsideTriangleV( vec3 intersection, vec3 v0, vec3 v1, vec3 v2)
{
	vec3 n = getNormalToTriangle(v0,v1,v2);
	if( (((v1 - v0)^(intersection - v0)) * n) <= 0) return false;
	if( (((v2 - v1)^(intersection - v1)) * n) <= 0) return false;
	if( (((v0 - v2)^(intersection - v2)) * n) <= 0) return false;
	return false;
}
//-----------------------------------------------------------------------------
// Name : isPointInsideTriangleFast
//-----------------------------------------------------------------------------
bool isPointInsideTriangleFast( vec3 intersection, vec3 v0, vec3 v1, vec3 v2, vec3 n)
{
	if( (((v1 - v0)^(intersection - v0)) * n) <= 0) return false;
	if( (((v2 - v1)^(intersection - v1)) * n) <= 0) return false;
	if( (((v0 - v2)^(intersection - v2)) * n) <= 0) return false;
	return false;
}

/////	¬ÓÁ‚‡˘‡ÂÚ ÌÓÏ‡Î¸ ÔÓÎË„ÓÌ‡
vec3 getNormalToTriangle( vec3 v0, vec3 v1, vec3 v2)
{
	return ((v1 - v0) ^ (v2 - v0)).normalized();
}
vec3 getNormalToTriangle(vec3 triangle[])
{
	return ((triangle[2] - triangle[0]) ^ (triangle[1] - triangle[0])).normalized();
}

float planeDistance(vec3 n, vec3 p)
{
	return - ((n.x * p.x) + (n.y * p.y) + (n.z * p.z));	// ¬ÓÁ‚‡ÚËÏ ‰ËÒÚ‡ÌˆË˛
}
 
bool intersectedPlane(vec3 poly[], vec3 line[], vec3 &normal, float &originDistance)
{
	float distance1=0, distance2=0;	// ƒËÒÚ‡ÌˆËˇ 2ı ÚÓ˜ÂÍ ÎËÌËË
	normal = getNormalToTriangle(poly);		// –‡ÒÒ˜ËÚ˚‚‡ÂÏ ÌÓÏ‡Î¸ ÔÎÓÒÍÓÒÚË
	originDistance = planeDistance(normal, poly[0]);
	distance1 = ((normal.x * line[0].x)  +					// Ax +
		         (normal.y * line[0].y)  +					// Bx +
				 (normal.z * line[0].z)) + originDistance;	// Cz + D
 
	distance2 = ((normal.x * line[1].x)  +					// Ax +
		         (normal.y * line[1].y)  +					// Bx +
				 (normal.z * line[1].z)) + originDistance;	// Cz + D
	// œÓ‚ÂËÏ Ì‡ ÔÂÂÒÂ˜ÂÌËÂ
 	if(distance1 * distance2 >= 0)   return false;
 	return true;
}
	
// —ÎÂ‰Û˛¯‡ˇ ÙÛÌÍˆËˇ ‚ÓÁ‚‡˘‡ÂÚ Û„ÓÎ ÏÂÊ‰Û ‚ÂÍÚÓ‡ÏË
float angleBetweenVectors(vec3 v1, vec3 v2)
{
	float vectorsMagnitude = v1.len() * v2.len();
	float angle = acosf( v1*v2 / vectorsMagnitude );
	if(_isnan(angle))	return 0;
	return angle ;
}
 
/////////////////////////////////// INSIDE POLYGON \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////	œÓ‚ÂˇÂÚ, Ì‡ıÓ‰ËÚÒˇ ÎË ÚÓ˜Í‡ ‚ÌÛÚË ÔÓÎË„ÓÌ‡
bool insidePolygon(vec3 intersection, vec3 poly[], long VerticeCount)
{
	const double MATCH_FACTOR = 0.9999;
	double angle = 0.0;		// »ÌËˆË‡ÎËÁËÛÂÏ Û„ÓÎ
	vec3 vA, vB;			// ¬ÂÏÂÌÌ˚Â ‚ÂÍÚÓ˚
	for (int i = 0; i < VerticeCount; i++)	// œÓıÓ‰ËÏ ˆËÍÎÓÏ ÔÓ Í‡Ê‰ÓÈ ‚Â¯ËÌÂ Ë ÒÍÎ‡‰˚‚‡ÂÏ Ëı Û„Î˚
	{
		vA = poly[i] - intersection;// ¬˚˜ËÚ‡ÂÏ ÚÓ˜ÍÛ ÔÂÂÒÂ˜ÂÌËˇ ËÁ ÚÂÍÛ˘ÂÈ ‚Â¯ËÌ˚
		vB = poly[(i + 1) % VerticeCount] - intersection;
 		angle += angleBetweenVectors(vA, vB);	// Õ‡ıÓ‰ËÏ Û„ÓÎ ÏÂÊ‰Û 2Ïˇ ‚ÂÍÚÓ‡ÏË Ë ÒÍÎ‡‰˚‚‡ÂÏ Ëı ‚ÒÂ
	}
	if(angle >= (MATCH_FACTOR * (2.0 * PI)) )// ≈ÒÎË Û„ÓÎ >= 2PI (360 „‡‰ÛÒÓ‚)
		return true;// “Ó˜Í‡ Ì‡ıÓ‰ËÚÒˇ ‚ÌÛÚË ÔÓÎË„ÓÌ‡
	return false;		// »Ì‡˜Â - ÒÌ‡ÛÊË
}

///////////////////////////////// CLASSIFY SPHERE """"""""""\\*
/////	‚˚˜ËÒÎˇÂÚ ÔÓÎÓÊÂÌËÂ ÒÙÂ˚ ÓÚÌÓÒËÚÂÎ¸ÌÓ ÔÎÓÒÍÓÒÚË, ‡ Ú‡Í ÊÂ ‡ÒÒÚÓˇÌËÂ
int classifySphere(vec3 &center,
		vec3 &normal, vec3 &point, float radius, float &distance)
{
	float d = planeDistance(normal, point);
	distance = normal*center + d;
	float difference = fabsf(distance) - radius;
	if(isFloatConsiderZero(difference) || (difference<0))	return INTERSECTS;
	else if(distance > radius)		return FRONT;
	return BEHIND;
}

////////////////////////////// CLOSET POINT ON LINE """""""""""\*
/////	¬ÓÁ‚‡˘‡ÂÚ ÚÓ˜ÍÛ Ì‡ ÎËÌËË vA_vB, ÍÓÚÓ‡ˇ ·ÎËÊÂ ‚ÒÂ„Ó Í ÚÓ˜ÍÂ vPoint
vec3 closestPointOnLine(vec3 a, vec3 b, vec3 p)
{
	vec3 c = p - a;
	vec3 V = b - a;
	float d = V.len();
	V /= d;
	float t = V * c;
	
	// проверка на выход за границы отрезка
	if ( t < 0.0f )
		return a;
	if ( t > d )
		return b;

	// Вернем точку между a и b
	V *= t;
	return ( a + V );
}

///////////////////////////////// EDGE SPHERE COLLSIION """"""""""\\*
/////	ÓÔÂ‰ÂÎˇÂÚ, ÔÂÂÒÂÍ‡ÂÚ ÎË ÒÙÂ‡ Í‡ÍÓÂ-ÎË·Ó Â·Ó ÚÂÛ„ÓÎ¸ÌËÍ‡
bool edgeSphereCollision(vec3 center,
						vec3 polygon[], int VertexCount, float radius, vec3 &point, float &distance)
{
	for(int i = 0; i < VertexCount; i++)
	{
		point = closestPointOnLine(polygon[i], polygon[(i + 1) % VertexCount], center);
 		distance = (point-center).len();
		if(distance < radius)	return true;
	}
	return false;
}
 
////////////////////////////// SPHERE POLYGON COLLISION """""""""\\*
/////	‚ÓÁ‚‡˘‡ÂÚ true ÂÒÎË ÒÙÂ‡ ÔÂÂÒÂÍ‡ÂÚ ÔÂÂ‰‡ÌÌ˚È ÔÓÎË„ÓÌ.
bool spherePolygonCollision(vec3 polygon[],vec3 &center, int VertexCount, float radius)
{
	vec3 normal = getNormalToTriangle(polygon);
	float distance = 0.0f;
	int classification = classifySphere(center, normal, polygon[0], radius, distance);
	if( classification == INTERSECTS)
	{
		vec3 offset = normal * distance;
		vec3 position = center - offset;
		if(insidePolygon(position, polygon, VertexCount)) return true;	// ≈ÒÚ¸ ÔÂÂÒÂ˜ÂÌËÂ!
		else if(edgeSphereCollision((vec3)center, polygon, VertexCount, radius, position, distance))
				return true;// We collided! "And you doubted me..." - Sphere
	}
	return false;
}

////////////////////////////// SPHERE TRIANGLE COLLISION """""""""\\*
/////	‚ÓÁ‚‡˘‡ÂÚ true ÂÒÎË ÒÙÂ‡ ÔÂÂÒÂÍ‡ÂÚ ÔÂÂ‰‡ÌÌ˚È ÔÓÎË„ÓÌ.
bool sphereTriangleCollision(vec3 v1, vec3 v2, vec3 v3, vec3 center, float radius)
{
	vec3 normal = getNormalToTriangle(v1,v2,v3);
	float distance = 0.0f;
	int classification = classifySphere(center, normal, v1, radius, distance);
	if( classification == INTERSECTS)
	{
		vec3 offset = normal * distance;
		vec3 position = center - offset;
		if(isPointInsideTriangleV(position, v1,v2,v3)) return true;	// ≈ÒÚ¸ ÔÂÂÒÂ˜ÂÌËÂ!
		else if(edgeSphereCollision(center, &v1, 3, radius,position,distance))
				return true;// We collided! "And you doubted me..." - Sphere
	}
	return false;
}

////////////////////////////// SPHERE TRIANGLE COLLISION """""""""\\*
/////	‚ÓÁ‚‡˘‡ÂÚ true ÂÒÎË ÒÙÂ‡ ÔÂÂÒÂÍ‡ÂÚ ÔÂÂ‰‡ÌÌ˚È ÔÓÎË„ÓÌ.
bool sphereTriangleCollision2(vec3 v1, vec3 v2, vec3 v3, vec3 center, float radius, vec3 &point, vec3 &normal, vec3 &offset)
{
	vec3 tri[3]={v1,v2,v3};
	normal = getNormalToTriangle(v1,v2,v3);
	float distance = 0.0f;
	int classification = classifySphere(center, normal, v1, radius, distance);
	if( classification == INTERSECTS)
	{
		offset = normal * distance;
		point = center - offset;
		if(isPointInsideTriangle( point, v1, v2, v3 ))
		{
			offset = getCollisionOffset(normal, radius, distance);
			return true;
		}
		///*
		else
		{
			if(edgeSphereCollision(center, tri, 3, radius, point, distance))
			{
				normal = (center - point) / distance;
				offset = getCollisionOffset(normal, radius, distance);
				return true;
			}
		}
		//*/
	}
	return false;
}

///////////////////////////////// GET COLLISION OFET \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////	This returns the offset to move the center of the sphere off the collided polygon
vec3 getCollisionOffset(vec3 &normal, float radius, float distance)
{
	if(distance > 0) return normal * (radius - distance);
	return normal * -(radius + distance);
}

float raySphereIntersection(vec3 rO, vec3 rV, vec3 sO, float sR) 
{
   vec3 Q = sO-rO;
   float c = Q.len();
   float v = Q * rV;
   float d = sR*sR - (c*c - v*v);

   // If there was no intersection, return -1
   if (d < 0.0f) return (-1.0f);

   // Return the distance to the [first] intersecting point
   return (v - fsqrt(d));
}

void calcTriangleTangentBasis( 	const vec3& E, const vec3& F, const vec3& G,
								const vec2& texE, const vec2& texF, const vec2& texG,
								vec3& tangentX, vec3& tangentY )
{
	vec3 P = F - E;
	vec3 Q = G - E;
	float s1 = texF.x - texE.x;
	float t1 = texF.y - texE.y;
	float s2 = texG.x - texE.x;
	float t2 = texG.y - texE.y;
	float pqMatrix[2][3];
	pqMatrix[0][0] = P[0];
	pqMatrix[0][1] = P[1];
	pqMatrix[0][2] = P[2];
	pqMatrix[1][0] = Q[0];
	pqMatrix[1][1] = Q[1];
	pqMatrix[1][2] = Q[2];
	float temp = 1.0f / maxValue( s1 * t2 - s2 * t1, EPSILON );
	float stMatrix[2][2];
	stMatrix[0][0] =  t2 * temp;
	stMatrix[0][1] = -t1 * temp;
	stMatrix[1][0] = -s2 * temp;
	stMatrix[1][1] =  s1 * temp;
	float tbMatrix[2][3];
	// stMatrix * pqMatrix
	tbMatrix[0][0] = stMatrix[0][0] * pqMatrix[0][0] + stMatrix[0][1] * pqMatrix[1][0];
	tbMatrix[0][1] = stMatrix[0][0] * pqMatrix[0][1] + stMatrix[0][1] * pqMatrix[1][1];
	tbMatrix[0][2] = stMatrix[0][0] * pqMatrix[0][2] + stMatrix[0][1] * pqMatrix[1][2];
	tbMatrix[1][0] = stMatrix[1][0] * pqMatrix[0][0] + stMatrix[1][1] * pqMatrix[1][0];
	tbMatrix[1][1] = stMatrix[1][0] * pqMatrix[0][1] + stMatrix[1][1] * pqMatrix[1][1];
	tbMatrix[1][2] = stMatrix[1][0] * pqMatrix[0][2] + stMatrix[1][1] * pqMatrix[1][2];
	
	tangentX = vec3( tbMatrix[0][0], tbMatrix[0][1], tbMatrix[0][2] );
	tangentY = vec3( tbMatrix[1][0], tbMatrix[1][1], tbMatrix[1][2] );

	tangentX.safeNormalize();
	tangentY.safeNormalize();
}

// ортогонализация векторов методом Грама-Шмидта
vec3 orthogonalize( const vec3& v1, const vec3& v2 )
{
	vec3 v2ProjV1 = closestPointOnLine( v1, -v1, v2 );
	vec3 res = v2 - v2ProjV1;
	res.normalize();
	return res;
}

} //namespace Math {

} //namespace Squirrel {