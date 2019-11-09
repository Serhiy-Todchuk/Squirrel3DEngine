// QuatSlerper.h: interface for the QuatSlerper class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "quat.h"

namespace Squirrel {

namespace Math {

/**
 Perform Spherical Linear Interpolation of the quaternions,
 return unit length quaternion 
*/

class QuatSlerper  
{
    quat mQuat1, mQuat2;
    float mOmega;
public:
	QuatSlerper(){}
	~QuatSlerper(){}
	
    inline void setupFromUnit(const quat& q1, const quat& q2){
        mQuat1 = q1;
        mQuat2 = q2;
		if(mQuat1 == mQuat2) return;
        float dot = mQuat1 * mQuat2;
        if(dot < 0 )
        {
            dot *= -1;
            mQuat2  *= -1;
        }
        if(dot > 0.9999f ) dot = 0.9999f;
        mOmega = acosf( dot );
        float invSine = (float)(1.0/fsin( mOmega ));
        mQuat1 *= invSine;
        mQuat2 *= invSine;
    }

    inline void setup(const quat& q1, const quat& q2)
	{
        mQuat1 = q1;
        mQuat1.normalize();
        mQuat2 = q2;
        mQuat2.normalize();
        setupFromUnit(mQuat1, mQuat2);
    }

    inline quat interpolate( float t )
	{
		if(mQuat1 == mQuat2) return mQuat1;
		return quat( mQuat1*fsin((1.0f-t)*mOmega) + mQuat2*fsin(t*mOmega) );
    }

};

} //namespace Math {

} //namespace Squirrel {
