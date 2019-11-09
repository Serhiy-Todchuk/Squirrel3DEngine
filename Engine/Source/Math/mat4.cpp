#include "mat4.h"

namespace Squirrel {

namespace Math {

float mat4::minor( const size_t r0, const size_t r1, const size_t r2,
					const size_t c0, const size_t c1, const size_t c2 ) const
{
    return (*this)[ r0 ][ c0 ] * ( (*this)[ r1 ][ c1 ] * (*this)[ r2 ][ c2 ] - (*this)[ r2 ][ c1 ] * (*this)[ r1 ][ c2 ] ) -
           (*this)[ r0 ][ c1 ] * ( (*this)[ r1 ][ c0 ] * (*this)[ r2 ][ c2 ] - (*this)[ r2 ][ c0 ] * (*this)[ r1 ][ c2 ] ) +
           (*this)[ r0 ][ c2 ] * ( (*this)[ r1 ][ c0 ] * (*this)[ r2 ][ c1 ] - (*this)[ r2 ][ c0 ] * (*this)[ r1 ][ c1 ] );
}

mat4 mat4::adjoint() const
{
	return mat4( minor( 1, 2, 3, 1, 2, 3 ),
				-minor( 0, 2, 3, 1, 2, 3 ),
				 minor( 0, 1, 3, 1, 2, 3 ),
				-minor( 0, 1, 2, 1, 2, 3 ),

				-minor( 1, 2, 3, 0, 2, 3 ),
				 minor( 0, 2, 3, 0, 2, 3 ),
				-minor( 0, 1, 3, 0, 2, 3 ),
				 minor( 0, 1, 2, 0, 2, 3 ),

				 minor( 1, 2, 3, 0, 1, 3 ),
				-minor( 0, 2, 3, 0, 1, 3 ),
				 minor( 0, 1, 3, 0, 1, 3 ),
				-minor( 0, 1, 2, 0, 1, 3 ),

				-minor( 1, 2, 3, 0, 1, 2 ),
				 minor( 0, 2, 3, 0, 1, 2 ),
				-minor( 0, 1, 3, 0, 1, 2 ),
				 minor( 0, 1, 2, 0, 1, 2 ) );
}

float mat4::determinant() const
{
    return (*this)[ 0 ][ 0 ] * minor( 1, 2, 3, 1, 2, 3 ) -
           (*this)[ 0 ][ 1 ] * minor( 1, 2, 3, 0, 2, 3 ) +
           (*this)[ 0 ][ 2 ] * minor( 1, 2, 3, 0, 1, 3 ) -
           (*this)[ 0 ][ 3 ] * minor( 1, 2, 3, 0, 1, 2 );
}

mat4 mat4::inverse() const
{
    return adjoint() * ( 1.0f / determinant() );
}

mat4 mat4::transposed(void) const
{
	return mat4(x.x, y.x, z.x, w.x,
	            x.y, y.y, z.y, w.y,
	            x.z, y.z, z.z, w.z,
	            x.w, y.w, z.w, w.w);
}

vec3 mat4::extractScale() const
{
	vec3 col1( (*this)[ 0 ][ 0 ], (*this)[ 1 ][ 0 ], (*this)[ 2 ][ 0 ] );
	vec3 col2( (*this)[ 0 ][ 1 ], (*this)[ 1 ][ 1 ], (*this)[ 2 ][ 1 ] );
	vec3 col3( (*this)[ 0 ][ 2 ], (*this)[ 1 ][ 2 ], (*this)[ 2 ][ 2 ] );

	float scaleX = col1.len();
	float scaleY = col2.len();
	float scaleZ = col3.len();

	return vec3( scaleX, scaleY, scaleZ );
}

mat4 mat4::RotX(float c, float s)
{
	return mat4(1.0f, 0.0f, 0.0f, 0.0f,
	            0.0f, +c,   +s,   0.0f,
	            0.0f, -s,   +c,   0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}

mat4 mat4::RotY(float c, float s)
{
	return mat4(+c,   0.0f, -s,   0.0f,
	            0.0f, 1.0f, 0.0f, 0.0f,
	            +s,   0.0f, +c,   0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}

mat4 mat4::RotZ(float c, float s)
{
	return mat4(+c,  +s,    0.0f, 0.0f,
	            -s,  +c,    0.0f, 0.0f,
	            0.0f, 0.0f, 1.0f, 0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}

mat4 mat4::Scale(vec3 scl)
{
	return mat4(scl.x, 0.0f,  0.0f,  0.0f,
	            0.0f,  scl.y, 0.0f,  0.0f,
	            0.0f,  0.0f,  scl.z, 0.0f,
	            0.0f,  0.0f,  0.0f,  1.0f);
}

mat4 mat4::Translate(vec3 pos)
{
	return mat4(1.0f, 0.0f, 0.0f, pos.x,
	            0.0f, 1.0f, 0.0f, pos.y,
	            0.0f, 0.0f, 1.0f, pos.z,
	            0.0f, 0.0f, 0.0f, 1.0f);
}

mat4 mat4::RotAxis(vec3 axis, float angle)
{
	mat4 r;
	r.identity();

	float c = fcos(angle);
	float s = fsin(angle);
	float t = 1.0f - c;

	r.x.x = c + axis.x*axis.x*t;
	r.y.y = c + axis.y*axis.y*t;
	r.z.z = c + axis.z*axis.z*t;

	float tmp1 = axis.x*axis.y*t;
	float tmp2 = axis.z*s;
	r.y.x = tmp1 + tmp2;
	r.x.y = tmp1 - tmp2;
	tmp1 = axis.x*axis.z*t;
	tmp2 = axis.y*s;
	r.z.x = tmp1 - tmp2;
	r.x.z = tmp1 + tmp2;    tmp1 = axis.y*axis.z*t;
	tmp2 = axis.x*s;
	r.z.y = tmp1 + tmp2;
	r.y.z = tmp1 - tmp2;

	return r;
}

mat4 mat4::Euler(float yaw, float pitch, float roll)
{
	float cy = fcos(yaw);
	float sy = fsin(yaw);
	float cp = fcos(pitch);
	float sp = fsin(pitch);
	float cr = fcos(roll);
	float sr = fsin(roll);
	
	return mat4(sp*sr*sy + cr*cy,	cp*sr,	cy*sp*sr - cr*sy,	0,
	            cr*sp*sy - cy*sr,	cp*cr,	cr*cy*sp + sr*sy,	0,
	            cp*sy,				- sp,	cp*cy,				0,
				0,					0,		0,					1);
}

mat4 mat4::Transform(vec3 pos, mat3 rot, vec3 scl)
{
    // Ordering:
    //    1. Scale
    //    2. Rotate
    //    3. Translate

	mat4 m;

    // Set up final matrix with scale, rotation and translation
    m[0][0] = scl.x * rot[0][0]; m[0][1] = scl.y * rot[0][1]; m[0][2] = scl.z * rot[0][2]; m[0][3] = pos.x;
    m[1][0] = scl.x * rot[1][0]; m[1][1] = scl.y * rot[1][1]; m[1][2] = scl.z * rot[1][2]; m[1][3] = pos.y;
    m[2][0] = scl.x * rot[2][0]; m[2][1] = scl.y * rot[2][1]; m[2][2] = scl.z * rot[2][2]; m[2][3] = pos.z;

    // No projection term
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;

	return m;
}

mat4 mat4::Transform(vec3 pos, vec3 scl)
{
	// Ordering:
	//    1. Scale
	//    2. Translate
	
	mat4 m;
	
	// Set up final matrix with scale, rotation and translation
	m[0][0] = scl.x;	m[0][1] = 0;		m[0][2] = 0;		m[0][3] = pos.x;
	m[1][0] = 0;		m[1][1] = scl.y;	m[1][2] = 0;		m[1][3] = pos.y;
	m[2][0] = 0;		m[2][1] = 0;		m[2][2] = scl.z;	m[2][3] = pos.z;
	
	// No projection term
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
	
	return m;
}
	
	
} //namespace Math {

} //namespace Squirrel {
