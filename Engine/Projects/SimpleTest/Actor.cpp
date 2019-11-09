// Actor.cpp: implementation of the Actor class.
//
//////////////////////////////////////////////////////////////////////

#include "Actor.h"
#include <Common/Data.h>
#include <Common/Input.h>
#include <World/SceneObject.h>
#include <World/Body.h>
#include <World/Terrain.h>
#include <Common/TimeCounter.h>

using namespace World;
using namespace Math;

const float turnSpeed = HALF_PI * 0.7f;
const float moveSpeed = 150;

Actor::Actor()
{
	SQREFL_SET_CLASS(Actor);

	mOrientAroundY			= 0;
	mSize					= vec3(0,0,0);

	mForwardDirection		= vec3::AxisZ();
	mModelForwardDirection	= vec3::AxisZ();

	mAnims					= NULL;

	mTurning	= false;
	mMoving		= false;
	mRuning		= false;
}

Actor::~Actor()
{

}

void Actor::updatePosition()
{
	quat dirRotation = quat().fromAxisAngle( vec3::AxisY(), mOrientAroundY );

	vec3 pos = mSceneObject->getLocalPosition();
	pos.y = World::Terrain::GetMain()->height( pos.x, pos.z );

	vec3 lSize = mSize * 0.94f;//magic number (TODO: make member variable)

	vec3 pos0 = pos - (vec3( lSize.x, 0, lSize.z ) * 0.5f);
	pos0.y = World::Terrain::GetMain()->height( pos0.x, pos0.z );

	vec3 posX = pos0 + vec3(lSize.x, 0, 0);
	vec3 posZ = pos0 + vec3(0, 0, lSize.z);

	posX.y = World::Terrain::GetMain()->height( posX.x, posX.z );
	posZ.y = World::Terrain::GetMain()->height( posZ.x, posZ.z );
	
	//a la "tangent basis"
	vec3 dirX = (posX - pos0).normalized();
	vec3 dirZ = (posZ - pos0).normalized();
	vec3 dirY = (dirZ ^ dirX).normalized();

	quat quaternion	= quat().fromRotationLook( vec3::AxisZ(), vec3::AxisY(), dirZ, dirY ) & dirRotation;

	mSceneObject->setLocalRotation( quaternion );
	mSceneObject->setLocalPosition( pos );
}

void Actor::turn(bool left)
{
	float deltaTime = TimeCounter::Instance().getDeltaTime();
	mTurning = true;
	mOrientAroundY += deltaTime * (left ? turnSpeed : -turnSpeed);
}

void Actor::move(bool forward, bool run)
{
	float deltaTime = TimeCounter::Instance().getDeltaTime();
	vec3 pos = mSceneObject->getLocalPosition();
	mMoving = true;
	pos += mForwardDirection * deltaTime * (forward ? moveSpeed : -moveSpeed);
	mSceneObject->setLocalPosition( pos );
}

void Actor::update()
{
	quat dirRotation = quat().fromAxisAngle( vec3::AxisY(), mOrientAroundY );
	mForwardDirection = dirRotation.apply( vec3::AxisZ() );

	if(mMoving)
	{
		if(mAnims->getLastAnimName() == "idle" || mAnims->getLastAnimName() == "walk")
			mAnims->playAnimNode("run");
	}
	else if(mTurning)
	{
		if(mAnims->getLastAnimName() == "idle")
			mAnims->playAnimNode("walk");
	}
	else
	{
		if(mAnims->getLastAnimName() == "walk" || mAnims->getLastAnimName() == "run")
			mAnims->stop();
	}

	if(!mAnims->isPlaying())
	{
		mAnims->playAnimNode("idle");
	}

	updatePosition();

	mTurning = false;
	mMoving = false;
	mRuning = false;
}

void Actor::awake()
{
	AABB aabb = mSceneObject->getAllAABB();

	//store size
	mSize = aabb.max -  aabb.min;

	mAnims = mSceneObject->getAnimations();
	mAnims->playAnimNode("idle");
}
