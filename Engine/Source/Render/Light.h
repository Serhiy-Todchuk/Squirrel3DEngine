#pragma once

#include <Common/Types.h>
#include <Common/common.h>
#include <common/LookAtObject.h>
#include <Math/vec4.h>
#include <string>
#include "Camera.h"
#include "macros.h"

namespace Squirrel {

namespace RenderData { 

class SQRENDER_API Light: 
	public LookAtObject
{
public:

	enum EType
	{
		ltOmni = 1,
		ltSpot,	
		ltDirectional,
		ltUnknown,
	};

public:
	Light(void);
	~Light(void);

	bool doesLit(const Math::AABB& bounds);

	void updateSpotFrustum();

	Math::vec4	getDiffuse		(void) const	{ return colorBytesToVec4(mDiffuse);	}
	Math::vec4	getSpecular		(void) const	{ return colorBytesToVec4(mSpecular);	}
	Math::vec4	getAmbient		(void) const	{ return colorBytesToVec4(mAmbient);	}

	EType		mLightType;

	tuple4ub	mDiffuse;
	tuple4ub	mSpecular;
	tuple4ub	mAmbient;

	float		mRadius;
	float		mInnerSpotAngle;
	float		mOuterSpotAngle;

	bool		mShadow;

private:

	Render::Camera mSpotFrustum;
};


}//namespace RenderData { 

}//namespace Squirrel {