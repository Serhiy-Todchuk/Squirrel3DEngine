#include "Light.h"

namespace Squirrel {

namespace RenderData { 


Light::Light(void)
{
	mDiffuse	= tuple4ub(255, 255, 255, 255);
	mSpecular	= tuple4ub(128, 128, 128, 255);
	mAmbient	= tuple4ub(0, 0, 0, 0);
	mRadius = 20.0f;
	mInnerSpotAngle = 80.0f;
	mOuterSpotAngle = 90.0f;
	mLightType = ltOmni;
	mShadow = false;

	mSpotFrustum.setType(Render::Camera::Perspective);	
}

Light::~Light(void)
{
	
}

bool Light::doesLit(const Math::AABB& bounds)
{
	switch(mLightType)
	{
	case ltDirectional:
		return true;//lits all objects
	case ltOmni:
		{
			Math::AABB lightBounds(getPosition(), getPosition());
			lightBounds.grow(mRadius);
			return bounds.intersects(lightBounds);
		}
	case ltSpot:
		return mSpotFrustum.isAABBIn(bounds);
	default: break;
	};

	return false;
}

void Light::updateSpotFrustum()
{
	if(mLightType != ltSpot)
		return;

	mSpotFrustum.set(getPosition(), getDirection());

	vec3 right = getDirection() ^ vec3(0,1,0);
	vec3 up = right ^ getDirection();
	mSpotFrustum.setUp(up);

	mSpotFrustum.buildProjection(mOuterSpotAngle * DEG2RAD, 1.0f, 0.1f, mRadius);
}

}//namespace RenderData { 

}//namespace Squirrel {

