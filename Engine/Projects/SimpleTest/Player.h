// Actor.h: interface for the Actor class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Actor.h"
#include <Render/Camera.h>

using namespace Squirrel;

class Player : public Actor
{
	float mCameraDistance;
	float mCameraAngle;

	Render::Camera * mCamera;

	Math::vec3 mCameraVelocity;

public:
	Player();
	virtual ~Player();

protected:
	virtual void awake();
	virtual void update();

private:
	void updateCamera(float deltaTime);
};

