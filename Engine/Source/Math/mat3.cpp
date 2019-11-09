#include "mat3.h"

namespace Squirrel {

namespace Math {

float mat3::det(void) const
{
	return x.y*y.z*z.x - x.z*y.y*z.x + x.z*y.x*z.y +
	       x.x*y.y*z.z - x.x*y.z*z.y - x.y*y.x*z.z ;
}

mat3 mat3::transposed(void) const
{
	return mat3(x.x, y.x, z.x,
	            x.y, y.y, z.y,
	            x.z, y.z, z.z);
}

mat3 mat3::inverse(void) const
{
	float d = (*this).det();
	
	return mat3(y.y*z.z - y.z*z.y, x.z*z.y - x.y*z.z, x.y*y.z - x.z*y.y,
	            y.z*z.x - y.x*z.z, x.x*z.z - x.z*z.x, x.z*y.x - x.x*y.z,
	            y.x*z.y - y.y*z.x, x.y*z.x - x.x*z.y, x.x*y.y - x.y*y.x) / d;
}

mat3 mat3::inverse2(void) const
{
	return mat3(y.y*z.z - y.z*z.y, x.z*z.y - x.y*z.z, x.y*y.z - x.z*y.y,
	            y.z*z.x - y.x*z.z, x.x*z.z - x.z*z.x, x.z*y.x - x.x*y.z,
	            y.x*z.y - y.y*z.x, x.y*z.x - x.x*z.y, x.x*y.y - x.y*y.x);
}

mat3 mat3::RotX(float c, float s)
{
	return mat3(1.0f, 0.0f, 0.0f, 0.0f, c, s, 0.0f, -s, c);
}

mat3 mat3::RotY(float c, float s)
{
	return mat3(c, 0.0f, -s, 0.0f, 1.0f, 0.0f, s, 0.0f, c);
}

mat3 mat3::RotZ(float c, float s)
{
	return mat3(c, s, 0.0f, -s, c, 0.0f, 0.0f, 0.0f, 1.0f);
}

mat3 mat3::RotAxis(vec3 axis, float angle)
{
	mat3 r;
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

mat3 mat3::Scale(vec3 scl)
{
	mat3 r;
	r.identity();
	r.x.x = scl.x;
	r.y.y = scl.y;
	r.z.z = scl.z;

	return r;
}

mat3 mat3::Euler(float yaw, float pitch, float roll)
{
	float cy = fcos(yaw);
	float sy = fsin(yaw);
	float cp = fcos(pitch);
	float sp = fsin(pitch);
	float cr = fcos(roll);
	float sr = fsin(roll);
	
	return mat3(sp*sr*sy + cr*cy, cp*sr, cy*sp*sr - cr*sy,
	            cr*sp*sy - cy*sr, cp*cr, cr*cy*sp + sr*sy,
	            cp*sy, - sp, cp*cy);
}

vec3 mat3::extractEulerAngles() const
{
	vec3 angles;
	// Assuming the angles are in radians.
	if (y.x > 0.998) { // singularity at north pole
		angles.x = 0;
		angles.y = atan2f(x.z,z.z);
		angles.z = HALF_PI;
		return angles;
	}
	if (y.x < -0.998) { // singularity at south pole
		angles.x = 0;
		angles.y = atan2f(x.z,z.z);
		angles.z = -HALF_PI;
		return angles;
	}
	angles.x = atan2f(-y.z,y.y);
	angles.y = atan2f(-z.x,x.x);
	angles.z = asinf(y.x);
	return angles;
}

void mat3::orthonormalize ()
{
	mat3 &matrix = (*this);
//-----------------------------------------------------------------------from ogre
	/*

    // Algorithm uses Gram-Schmidt orthogonalization.  If 'this' matrix is
    // M = [m0|m1|m2], then orthonormal output matrix is Q = [q0|q1|q2],
    //
    //   q0 = m0/|m0|
    //   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
    //   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
    //
    // where |V| indicates length of vector V and A*B indicates dot
    // product of vectors A and B.

    // compute q0
    float fInvLength = 1.0f/fsqrt(m[0][0]*m[0][0]
        + m[1][0]*m[1][0] +
        m[2][0]*m[2][0]);

    m[0][0] *= fInvLength;
    m[1][0] *= fInvLength;
    m[2][0] *= fInvLength;

    // compute q1
    float fDot0 =
        m[0][0]*m[0][1] +
        m[1][0]*m[1][1] +
        m[2][0]*m[2][1];

    m[0][1] -= fDot0*m[0][0];
    m[1][1] -= fDot0*m[1][0];
    m[2][1] -= fDot0*m[2][0];

    fInvLength = 1.0f/fsqrt(m[0][1]*m[0][1] +
        m[1][1]*m[1][1] +
        m[2][1]*m[2][1]);

    m[0][1] *= fInvLength;
    m[1][1] *= fInvLength;
    m[2][1] *= fInvLength;

    // compute q2
    float fDot1 =
        m[0][1]*m[0][2] +
        m[1][1]*m[1][2] +
        m[2][1]*m[2][2];

    fDot0 =
        m[0][0]*m[0][2] +
        m[1][0]*m[1][2] +
        m[2][0]*m[2][2];

    m[0][2] -= fDot0*m[0][0] + fDot1*m[0][1];
    m[1][2] -= fDot0*m[1][0] + fDot1*m[1][1];
    m[2][2] -= fDot0*m[2][0] + fDot1*m[2][1];

    fInvLength = 1.0f/fsqrt(m[0][2]*m[0][2] +
        m[1][2]*m[1][2] +
        m[2][2]*m[2][2]);

    m[0][2] *= fInvLength;
    m[1][2] *= fInvLength;
    m[2][2] *= fInvLength;
	*/

	//taken from Benjamin Jurke from flipcode archives

	float fLength = fsqrt((matrix.x.x * matrix.x.x) + (matrix.y.x * matrix.y.x) + (matrix.z.x * matrix.z.x));
	if (fLength > 0.0f)
	{
		fLength = 1.0f / fLength;
		matrix.x.x *= fLength;
		matrix.y.x *= fLength;
		matrix.z.x *= fLength;
	}

	fLength = fsqrt((matrix.x.y * matrix.x.y) + (matrix.y.y * matrix.y.y) + (matrix.z.y * matrix.z.y));
	if (fLength > 0.0f)
	{
		fLength = 1.0f / fLength;
		matrix.x.y *= fLength;
		matrix.y.y *= fLength;
		matrix.z.y *= fLength;
	}

	matrix.x.z = (matrix.y.x * matrix.z.y) - (matrix.z.x * matrix.y.y);
	matrix.y.z = (matrix.z.x * matrix.x.y) - (matrix.x.x * matrix.z.y);
	matrix.z.z = (matrix.x.x * matrix.y.y) - (matrix.y.x * matrix.x.y);
}

} //namespace Math {

} //namespace Squirrel {
