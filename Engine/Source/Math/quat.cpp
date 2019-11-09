#include "quat.h"

namespace Squirrel {

namespace Math {

quat::quat()
{
	x = y = z = 0, w = 1;
}

quat::quat(const float& _x, const float& _y, const float& _z, const float& _w)
{
	x = _x, y = _y, z = _z, w = _w;
}

quat::quat(const vec3& _v, const float& _w)
{
	x = _v.x, y = _v.y, z = _v.z, w = _w;
}

quat::quat(const vec4 &v)
{
	x = v.x, y = v.y, z = v.z, w = v.w;
}

quat::quat(const mat3 &m)
{
	fromRotationMatrix( m );
}

quat quat::conj() const
{
	return quat( -x, -y, -z, w );
}

void quat::renormalize()
{
	float len = 1 - x*x - y*y - z*z;
	w = (len < 1e-8) ? 0 : -fsqrt(len);
}

quat quat::operator&(const quat& q)
{
	return quat(
		w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y + y * q.w + z * q.x - x * q.z,
		w * q.z + z * q.w + x * q.y - y * q.x,
		w * q.w - x * q.x - y * q.y - z * q.z
	);
}

void quat::operator&=(const quat& q)
{
	(*this)=(*this)&q;
}


/*
inline void normalize()
{
	float	lenSq = len_squared();
	if ( lenSq > 1.0f - EPSILON )
	{
		float	invLen = 1.0f / lenSq;
		(*this)*=lenSq;
		w = 0.0f;
	}
	else
		w = (float) sqrt ( 1.0f - lenSq );
}
*/

// —оздать из угла и оси
quat quat::fromAxisAngle(vec3 axis, float angle)
{
	float scale = fsin(angle * 0.5f);
	set( axis * scale, fcos(angle * 0.5f));
	return *this;
}

// —оздать из угла
quat quat::fromPhysicAngle(vec3 ph)
{
	float angle = ph.len();
	if(angle>0) ph/=angle;
	else 	return *this;
	fromAxisAngle( ph, angle );
	return *this;
}

// —конвертировать в осевой угол
vec4 quat::toAxisAngle(vec4& dst) const
{
	float s=x*x+y*y+z*z;
	if (s>0)
	{
		dst.w = 2.0f * acosf(w);
		float inv_s=1/fsqrt(s);
		dst.x = x*inv_s;
		dst.y = y*inv_s;
		dst.z = z*inv_s;
	}
	else 
	{
		dst.set(vec3(0,0,1),0);
	}
	return dst;
}

// —конвертировать в угол
vec3 quat::toPhysicAngle(vec3& dst) const
{
	vec4 axisAngle;
	toAxisAngle(axisAngle);
	dst=axisAngle.getVec3()*axisAngle.w;
	return dst;
}

// ѕереназначить из углов Ёйлера, заданных координатами
quat quat::fromEulerAngle ( float yaw, float pitch, float roll )
{
	yaw   *= 0.5f;
	pitch *= 0.5f;
	roll  *= 0.5f;

	float	cx = fcos ( yaw	  );
	float	cy = fcos ( pitch );
	float	cz = fcos ( roll  );
	float	sx = fsin ( yaw   );
	float	sy = fsin ( pitch );
	float	sz = fsin ( roll  );

	float	cc = cx * cx;
	float	cs = cx * sz;
	float	sc = sx * cz;
	float	ss = sx * sz;

	x = cy * sc - sy * cs;
	y = cy * ss + sy * cc;
	z = cy * cs - sy * sc;
	w = cy * cc + sy * ss;

	return *this;
}

// ѕереназначить из углов Ёйлера, заданных координатами
quat quat::fromEulerAngle ( vec3 e )
{
	return fromEulerAngle(e.x,e.y,e.z);
}

// —конвертировать в углы Ёйлера
vec3 quat::toEulerAngle(vec3 &dst) const
{
	float ww = w*w;
	float xx = x*x;
	float yy = y*y;
	float zz = z*z;
	float unit = xx + yy + zz + ww; // if normalised is one, otherwise is correction factor
	float test = x*y + z*w;
	if (test > 0.49999f*unit) { // singularity at north pole
		dst.x = 0;
		dst.y = 2 * atan2f(x,w);
		dst.z = PI/2;
		return dst;
	}
	if (test < -0.49999f*unit) { // singularity at south pole
		dst.z = 0;
		dst.y = -2 * atan2f(x,w);
		dst.z = -PI/2;
		return dst;
	}
	dst.x = atan2f(2*x*w-2*y*z , -xx + yy - zz + ww);
    dst.y = atan2f(2*y*w-2*x*z , xx - yy - zz + ww);
	dst.z = asinf(2*test/unit);
	//dst.z = atan2f(2*x*y+2*w*z, 1.0f-(2*yy+2*zz));
	return dst;

	/*
	dst.z = atan2f(2*(x*y + w*z), w*w + x*x - y*y - z*z);
	dst.x = atan2f(2*(y*z + w*x), w*w - x*x - y*y + z*z);
	dst.y = asinf(-2*(x*z - w*y));
	*/
	/*
	{
		// roll = atan2(localx.y, localx.x)
		// pick parts of xAxis() implementation that we need
		//float fTx  = 2.0*x;
		float fTy  = 2.0f*y;
		float fTz  = 2.0f*z;
		float fTwz = fTz*w;
		float fTxy = fTy*x;
		float fTyy = fTy*y;
		float fTzz = fTz*z;

		// Vector3(1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);

		dst.z = atan2f(fTxy+fTwz, 1.0f-(fTyy+fTzz));
	}

	{
		// pitch = atan2(localy.z, localy.y)
		// pick parts of yAxis() implementation that we need
		float fTx  = 2.0f*x;
//		float fTy  = 2.0f*y;
		float fTz  = 2.0f*z;
		float fTwx = fTx*w;
		float fTxx = fTx*x;
		float fTyz = fTz*y;
		float fTzz = fTz*z;

		// Vector3(fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx);
		dst.x = atan2f(fTyz+fTwx, 1.0f-(fTxx+fTzz));
	}

	{
		// yaw = atan2(localz.x, localz.z)
		// pick parts of zAxis() implementation that we need
		float fTx  = 2.0f*x;
		float fTy  = 2.0f*y;
		float fTz  = 2.0f*z;
		float fTwy = fTy*w;
		float fTxx = fTx*x;
		float fTxz = fTz*x;
		float fTyy = fTy*y;

		// Vector3(fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy));

		dst.y = atan2f(fTxz+fTwy, 1.0f-(fTxx+fTyy));
	}
	*/
}

quat& quat::fromRotationMatrixNV( const mat3 & m )
{
    float tr, s;
    int i, j, k;
    const int nxt[3] = { 1, 2, 0 };

    tr = m[0][0] + m[1][1] + m[2][2];

    if ( tr > 0.0f )
    {
        s = fsqrt( tr /*+m(3,3)*/ );
        w = s * 0.5f;
        s = 0.5f / s;

        x = ( m[1][2] - m[2][1] ) * s;
        y = ( m[2][0] - m[0][2] ) * s;
        z = ( m[0][1] - m[1][0] ) * s;
    }
    else
    {
        i = 0;
        if ( m[1][1] > m[0][0] )
            i = 1;

        if ( m[2][2] > m[i][i] )
            i = 2;

		float *_array = &x;

        j = nxt[i];
        k = nxt[j];

        s = fsqrt( ( m[i][j] - ( m[j][j] + m[k][k] )) + 1.0f );

        _array[i] = s * 0.5f;
        s = 0.5f / s;

        _array[3] = ( m[j][k] - m[k][j] ) * s;
        _array[j] = ( m[i][j] + m[j][i] ) * s;
        _array[k] = ( m[i][k] + m[k][i] ) * s;
    }

    return *this;
}

//-----------------------------------------------------------------------
quat quat::fromRotationMatrix ( const mat3& kRot )
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    float fTrace = kRot[0][0] + kRot[1][1] + kRot[2][2];
    float fRoot;

    if ( fTrace > 0.0 )
    {
        // |w| > 1/2, may as well choose w > 1/2
        fRoot = fsqrt( fTrace + 1.0f );  // 2w
        w = 0.5f * fRoot;
        fRoot = 0.5f / fRoot;  // 1/(4w)
        x = ( kRot[ 2 ][ 1 ] - kRot[ 1 ][ 2 ] ) * fRoot;
        y = ( kRot[ 0 ][ 2 ] - kRot[ 2 ][ 0 ] ) * fRoot;
        z = ( kRot[ 1 ][ 0 ] - kRot[ 0 ][ 1 ] ) * fRoot;
    }
    else
    {
        // |w| <= 1/2
        static size_t s_iNext[ 3 ] = { 1, 2, 0 };
        size_t i = 0;

        if ( kRot[ 1 ][ 1 ] > kRot[ 0 ][ 0 ] )
            i = 1;

        if ( kRot[ 2 ][ 2 ] > kRot[ i ][ i ] )
            i = 2;

        size_t j = s_iNext[ i ];
        size_t k = s_iNext[ j ];

        fRoot = fsqrt( kRot[ i ][ i ] - kRot[ j ][ j ] - kRot[ k ][ k ] + 1.0f );

        float* apkQuat[ 3 ] = { &x, &y, &z };

        *apkQuat[ i ] = 0.5f * fRoot;

        fRoot = 0.5f / fRoot;

        w = ( kRot[ k ][ j ] - kRot[ j ][ k ] ) * fRoot;

        *apkQuat[ j ] = ( kRot[ j ][ i ] + kRot[ i ][ j ] ) * fRoot;

        *apkQuat[ k ] = ( kRot[ k ][ i ] + kRot[ i ][ k ] ) * fRoot;
    }

	return *this;
}

mat3 quat::toRotationMatrix() const
{
	mat3 m;

    float sqw = w*w;
    float sqx = x*x;
    float sqy = y*y;
    float sqz = z*z;

    // invs (inverse square length) is only required if quaternion is not already normalised
    float invs = 1.0f / (sqx + sqy + sqz + sqw);
    m.x.x = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
    m.y.y = (-sqx + sqy - sqz + sqw)*invs ;
    m.z.z = (-sqx - sqy + sqz + sqw)*invs ;
    
    float tmp1 = x*y;
    float tmp2 = z*w;
    m.y.x = 2.0f * (tmp1 + tmp2)*invs ;
    m.x.y = 2.0f * (tmp1 - tmp2)*invs ;
    
    tmp1 = x*z;
    tmp2 = y*w;
    m.z.x = 2.0f * (tmp1 - tmp2)*invs ;
    m.x.z = 2.0f * (tmp1 + tmp2)*invs ;
    tmp1 = y*z;
    tmp2 = x*w;
    m.z.y = 2.0f * (tmp1 + tmp2)*invs ;
    m.y.z = 2.0f * (tmp1 - tmp2)*invs ;      

	return m;
}


quat quat::fromRotationBetween( const vec3 & rotateFrom, const vec3 & rotateTo )
{
    vec3 p1, p2;
	float alpha;

    p1 = rotateFrom.normalized();
    p2 = rotateTo.normalized();

    alpha = p1 * p2;

    if( alpha == 1.0f ) {
        *this = quat(); 
        return *this; 
    }

    // ensures that the anti-parallel case leads to a positive dot
    if( alpha == -1.0f )
    {
        vec3 v;

        if(p1.x != p1.y || p1.x != p1.z)
            v = vec3(p1.y, p1.z, p1.x);
        else
            v = vec3(-p1.x, p1.y, p1.z);

        v -= p1 * (p1 * v);
		v.normalize();

        return fromAxisAngle(v, PI);
    }

	p1 = ( p1 ^ p2 ).normalized();  
    
    return fromAxisAngle(p1,acosf(alpha));
}

quat quat::fromRotationLook( const vec3& from_look, const vec3& from_up,
    const vec3& to_look, const vec3& to_up)
{
    quat r_look = quat().fromRotationBetween(from_look, to_look);

	vec3 rotated_from_up(r_look.apply( from_up ));

    quat r_twist = quat().fromRotationBetween(rotated_from_up, to_up);

    *this = r_twist;
    *this = (*this) & r_look;
    return *this;
}

//
// Quaternion multiplication with cartesian vector
// v' = q*v*q(star)
//
void quat::multVec( const vec3&src, vec3&dst ) const
{
    float v_coef = w * w - x * x - y * y - z * z;                     
    float u_coef = 2.0f * (src[0] * x + src[1] * y + src[2] * z);  
    float c_coef = 2.0f * w;                                       

    dst[0] = v_coef * src[0] + u_coef * x + c_coef * (y * src[2] - z * src[1]);
    dst[1] = v_coef * src[1] + u_coef * y + c_coef * (z * src[0] - x * src[2]);
    dst[2] = v_coef * src[2] + u_coef * z + c_coef * (x * src[1] - y * src[0]);
}

void quat::multVec( vec3& src_and_dst) const
{
    multVec(vec3(src_and_dst), src_and_dst);
}


quat quat::slerp(const quat& q, float t )
{
	float	dot = (*this)*q;
	// check wthere q1 and q2 are opposite
	if ( 1 + dot> EPSILON )
	{
		if ( 1 - dot > EPSILON )
		{
			float	omega   = acosf ( dot );
			float	invSine = 1.0f / fsin ( omega );
			float	scale1  = invSine * fsin ( ( 1.0f - t ) * omega );
			float	scale2  = invSine * fsin ( t * omega );
			return  quat(	((*this)*scale1) + (q*scale2) );
		}
		else
		{
			float	scale1 = 1.0f - t;
			float	scale2 = t;
			return  quat(	((*this)*scale1) + (q*scale2) );
		}
	}
	// quaternions are nearly opposite, create a perpendicual quaternion and slerp it
	float	scale1 = fsin ( ( 1.0f - t ) * PI );
	float	scale2 = fsin ( t * PI );
	return  quat ( 	scale1 * x + scale2 * ( -q.y ),
					scale1 * y + scale2 * ( +q.x ),
					scale1 * z + scale2 * ( -q.w ),
					scale1 * w + scale2 * ( +q.w ) );
}

// ¬озвращает вектор направлени€
vec3 quat::getDirVector() const
{
	return vec3(	2.0f*(x*z-w*y),
					2.0f*(y*z+w*x),
					1.0f-2.0f*(x*x+y*y)		);
}
/*
inline vec3 apply (const vec3 &v) const
{
	quat r(	v.x * w + v.z * y - v.y * z, 			
			v.y * w + v.x * z - v.z * x, 			
			v.z * w + v.y * x - v.x * y,
			v.x * x + v.y * y + v.z * z 
			);
	return vec3 (	w * r.x + x * r.w + y * r.z - z * r.y,
					w * r.y + y * r.w + z * r.x - x * r.z,
					w * r.z + z * r.w + x * r.y - y * r.x	);
}
*/
//*
vec3 quat::apply( const vec3& v ) const
{
	quat p ( v, 0 );
	quat qConj ( -x, -y, -z, w );
	p  = (quat(*this)) & p & qConj;
	return vec3 ( p.x, p.y, p.z );
}
//*/

} //namespace Math {

} //namespace Squirrel {
