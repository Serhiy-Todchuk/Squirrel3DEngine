// Light.cpp: implementation of the Light class.
//
//////////////////////////////////////////////////////////////////////

#include "Light.h"

namespace Squirrel {
namespace World { 

const Math::vec3 Light::sDefaultDirection(0, 0, 1);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SQREFL_REGISTER_CLASS_SEED(World::Light, WorldLight);

Light * Light::Create(SceneObjectsContainer * dst)
{
	Light * light = new Light;
	dst->addSceneObject(light);
	return light;
}
	
Light::Light()
{
	mTransformLight = true;

	SQREFL_SET_CLASS(World::Light);

	wrapAtomicField("LightType",		(int*)&mLight.mLightType);//TODO: enum wrapper
	wrapAtomicField("Diffuse",			&mLight.mDiffuse.x,		4);
	wrapAtomicField("Specular",			&mLight.mSpecular.x,	4);
	wrapAtomicField("Ambient",			&mLight.mAmbient.x,		4);
	wrapAtomicField("Radius",			&mLight.mRadius);
	wrapAtomicField("OuterSpotAngle",	&mLight.mOuterSpotAngle);
	wrapAtomicField("InnerSpotAngle",	&mLight.mInnerSpotAngle);
	wrapAtomicField("Shadow",			&mLight.mShadow);

	wrapAtomicField("TransformLight",	&mTransformLight);
}

Light::~Light()
{
}

void Light::render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	if(info.level >= rilLighting)
		renderQueue->put(&mLight);
}

void Light::update(float dtime)
{
	if(mTransformLight)
	{
		mLight.set(getPosition(), getRotation().apply(sDefaultDirection));
	}

	if(mLight.mLightType == Render::Light::ltSpot)
	{
		mLight.updateSpotFrustum();
	}
}

bool Light::isInCamera(Render::Camera * camera)
{
	AABB aabb;
	aabb.reset();
	switch(mLight.mLightType)
	{
	case Render::Light::ltDirectional:
		return true;//always in cam
	case Render::Light::ltOmni:
	case Render::Light::ltSpot:
		return camera->isSphereIn(getPosition(), mLight.mRadius, true);
	case Render::Light::ltUnknown:
	default:
		ASSERT(false);
	};

	return false;
}

void Light::calcAABB()
{
	vec3 pos = getPosition();
	
	if(mTransformLight)
	{
		mLight.set(pos, getRotation().apply(sDefaultDirection));
	}

	mAABB.setPoint(pos);
	mAABB.grow(mLight.mRadius);
}

}//namespace World { 
}//namespace Squirrel {
