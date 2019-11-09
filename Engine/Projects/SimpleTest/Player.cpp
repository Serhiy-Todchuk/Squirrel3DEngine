// Player.cpp: implementation of the Player class.
//
//////////////////////////////////////////////////////////////////////

#include "Player.h"
#include <Common/Data.h>
#include <Common/Input.h>
#include <World/SceneObject.h>
#include <World/Terrain.h>
#include <Common/TimeCounter.h>

using namespace Math;

float springConstant = 32.0f;
float dampingConstant = 8.0f;

const float maxPhysicalDeltaTime = 0.03f;

SQ_REGISTER_BEHAVIOUR(Player);

Player::Player()
{
	SQREFL_SET_CLASS(Player);

	mCameraDistance			= 150;
	mCameraAngle			= PI * 0.25f;

	mCamera					= 0;
	mCameraVelocity			= vec3(0,0,0);
}

Player::~Player()
{

}

void Player::updateCamera(float deltaTime)
{
	mCamera = Render::Camera::GetMainCamera();
	if(mCamera == 0) return;

	//camera spring connection

	vec3 pos = mSceneObject->getLocalPosition();

	vec3 camPos = mCamera->getPosition();

    // Calculate the new camera position. The 'idealPosition' is where thew
    // camera should be position. The camera should be positioned directly
    // behind the target at the required offset distance. What we're doing here
    // is rather than have the camera immediately snap to the 'idealPosition'
    // we slowly move the camera towards the 'idealPosition' using a spring
    // system.
    //
    // References:
    //   Stone, Jonathan, "Third-Person Camera Navigation," Game Programming
    //     Gems 4, Andrew Kirmse, Editor, Charles River Media, Inc., 2004.

    vec3 idealPosition = pos - (mForwardDirection * mCameraDistance);
	idealPosition.y = pos.y + (mCameraDistance * Math::fsin( mCameraAngle ));//fast rotation

    vec3 displacement = camPos - idealPosition;
    vec3 springAcceleration = (-displacement * springConstant) - 
        (mCameraVelocity * dampingConstant);

    mCameraVelocity += springAcceleration * deltaTime;
    camPos += mCameraVelocity * deltaTime;

	mCamera->setPosition( camPos );
	mCamera->setDirection( ((pos - camPos).normalized() + mForwardDirection).normalized() );
}

void Player::update()
{
	if(Input::Get()->isKeyPressed( Input::KeyA ))
	{
		turn(true);
	}
	else
	if(Input::Get()->isKeyPressed( Input::KeyD ))
	{
		turn(false);
	}
	if(Input::Get()->isKeyPressed( Input::KeyW ))
	{
		move(true);
	}
	else
	if(Input::Get()->isKeyPressed( Input::KeyS ))
	{
		move(false);
	}

	if(Input::Get()->isKeyDown( Input::Space ))
		mAnims->playAnimNode("jump");

	if(Input::Get()->isKeyDown( Input::Mouse0 ))
		mAnims->playAnimNode("attack");

	if(Input::Get()->isKeyDown( Input::Mouse1 ))
		mAnims->playAnimNode("attack2");

	Actor::update();

	//update camera

	float deltaTime = TimeCounter::Instance().getDeltaTime();

	int updateTimes = 1;
	float physicalDeltaTime = deltaTime;

	if(deltaTime < maxPhysicalDeltaTime)
	{
		updateTimes = Math::roundToBigger( maxPhysicalDeltaTime/deltaTime );
		physicalDeltaTime = deltaTime/updateTimes;
	}

	for(int i = 0; i < updateTimes; ++i)
		updateCamera(physicalDeltaTime);
}

void Player::awake()
{
	Actor::awake();
}
