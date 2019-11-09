#pragma once

#include <common/common.h>
#include <common/LookAtObject.h>
#include <Common/Data.h>
#include "SceneObject.h"
#include <Render/Light.h>

namespace Squirrel {
namespace World { 

using namespace Math;

class SQWORLD_API Light: 
	public SceneObject
{
	friend class Reflection::ObjectCreatorImpl<Light>;
	
	Light();
public:

	static Light * Create(SceneObjectsContainer * dst);
	~Light();

public:
	

	virtual void render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);
	virtual bool isInCamera(Render::Camera * camera);

	virtual void update(float dtime);

	bool doesLit(const Math::AABB& bounds);

	RenderData::Light& getLight() { return mLight; } 

	void setTransformLight(bool transform) { mTransformLight = transform; }

	//getters
	RenderData::Light::EType	getLightType(void) const	{return mLight.mLightType;	}
	tuple4ub	getDiffuse			(void) const	{return mLight.mDiffuse;	}
	tuple4ub	getSpecular			(void) const	{return mLight.mSpecular;	}
	tuple4ub	getAmbient			(void) const	{return mLight.mAmbient;	}
	float		getRadius			(void) const	{return mLight.mRadius;		}
	float		getOuterSpotRadius	(void) const	{return mLight.mOuterSpotAngle;}
	float		getInnerSpotRadius	(void) const	{return mLight.mInnerSpotAngle;}
	bool		getShadow			(void) const	{return mLight.mShadow;}

	//setters
	void		setLightType(RenderData::Light::EType lt)	{ mLight.mLightType	= lt;}
	void		setDiffuse	(tuple4ub v)		{ mLight.mDiffuse			= v;}
	void		setSpecular	(tuple4ub v)		{ mLight.mSpecular			= v;}
	void		setAmbient	(tuple4ub v)		{ mLight.mAmbient			= v;}
	void		setRadius	(float r)			{ mLight.mRadius			= r;}
	void		setInnerSpotRadius(float a)		{ mLight.mInnerSpotAngle	= a;}
	void		setOuterSpotRadius(float a)		{ mLight.mOuterSpotAngle	= a;}
	void		setShadow(bool sh)				{ mLight.mShadow		= sh;}

protected:

	virtual void calcAABB();

private:

	bool mTransformLight;

	Render::Camera mSpotFrustum;

	RenderData::Light mLight;

	static const Math::vec3 sDefaultDirection;
};


}//namespace World { 
}//namespace Squirrel {
