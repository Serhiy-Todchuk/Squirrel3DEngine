// HeightMapGen.h: interface for the HeightMapGen class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <common/common.h>
#include <Math/mathTypes.h>
#include <World/SceneObject.h>
#include <World/Behaviour.h>
#include <Resource/AnimationRunner.h>

using namespace Squirrel;

class Actor : public World::Behaviour
{
protected:

	float mOrientAroundY;
	Math::vec3 mSize;

	Math::vec3 mModelForwardDirection;
	Math::vec3 mForwardDirection;

	Resource::AnimationRunner * mAnims;

	bool mTurning;
	bool mMoving;
	bool mRuning;
	
protected:

	void turn(bool left);
	void move(bool forward, bool run = true);

public:
	Actor();
	virtual ~Actor();

protected:
	virtual void awake();
	virtual void update();

private:
	void updatePosition();
};

