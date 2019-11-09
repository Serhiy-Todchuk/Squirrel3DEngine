// FPSWalker.cpp: implementation of the FPSWalker class.
//
//////////////////////////////////////////////////////////////////////

#include "FPSWalker.h"
#include <Common/Data.h>
#include <Common/Input.h>
#include <World/SceneObject.h>
#include <World/Terrain.h>
#include <Common/TimeCounter.h>

using namespace Math;

SQ_REGISTER_BEHAVIOUR_SEED(FPSWalker, FPSWalker);

FPSWalker::FPSWalker()
{
	SQREFL_SET_CLASS(FPSWalker);

	mCamera			= NULL;
	mSpeed			= 5.5f;
	mMouseSensivity	= 1;
	mHeight			= 2.0f;

	mVelocity = Math::vec3(0, 0, 0);
}

FPSWalker::~FPSWalker()
{

}

void FPSWalker::update()
{
	float deltaTime = TimeCounter::Instance().getDeltaTime();

	mCamera = Render::Camera::GetMainCamera();
	if(mCamera == 0) return;

	if(Input::Get()->isMouseMoved())
	{
		tuple2i mousePos = Input::Get()->getMousePos();

		tuple2i mouseDiff = mousePos - mPrevMouse;

		mPrevMouse = mousePos;

		if(mouseDiff.x != 0)
		{
			float step = (float)mouseDiff.x * mMouseSensivity * DEG2RAD;
			mCamera->turnLRDir(-step);
		}
		if(mouseDiff.y != 0)
		{
			float step = (float)mouseDiff.y * mMouseSensivity * DEG2RAD;
			mCamera->turnUDDir(-step);
		}
	}

	float step = mSpeed * deltaTime;
	if(Input::Get()->isKeyPressed( Input::LShift ))
	{
		step *= 8;
	}

	if(Input::Get()->isKeyPressed( Input::KeyA ))
	{
		mCamera->moveRLPosDirXZ(step);
	}
	else
	if(Input::Get()->isKeyPressed( Input::KeyD ))
	{
		mCamera->moveRLPosDirXZ(-step);
	}
	if(Input::Get()->isKeyPressed( Input::KeyW ))
	{
		mCamera->movePosDirXZ(step);
	}
	else
	if(Input::Get()->isKeyPressed( Input::KeyS ))
	{
		mCamera->movePosDirXZ(-step);
	}

	if(Input::Get()->isKeyDown( Input::Space ))
	{
		mVelocity = Math::vec3(0, 26, 0);
	}

	if(Input::Get()->isKeyPressed( Input::Mouse1 ))
	{
		if(mGunSound)
		{
			mGunSound->setLocalPosition( vec3(0, -0.2, 0) );
			if(mGunSound->getSource()->getState() != Audio::ISource::PLAYING)
				mGunSound->getSource()->play();
		}
		if(mGunParticles)
		{
			mGunParticles->setLocalPosition( vec3(0, -0.2, 0) );
			mGunParticles->setStartVelocity( mCamera->getDirection() * 80 );
			mGunParticles->setEmit(true);
		}
	}
	else
	{
		if(mGunSound)
			mGunSound->getSource()->stop();
		if(mGunParticles)
			mGunParticles->setEmit(false);
	}

	mVelocity += vec3(0, -9.8f, 0) * deltaTime;

	if(mVelocity.y < -100) mVelocity.y = -100;

	vec3 pos = mCamera->getPosition();
	pos += mVelocity * deltaTime;

	//correct y-pos
	float minHeight = World::Terrain::GetMain()->height( pos.x, pos.z ) + mHeight;
	if(pos.y < minHeight)
		pos.y = minHeight;
	mCamera->setPosition(pos);
	
	mSceneObject->setLocalPosition(pos);
}

void FPSWalker::awake()
{
	mPrevMouse = Input::Get()->getMousePos();
}

void FPSWalker::initGun(const char_t * bulletTexture, const char_t * bulletSound)
{
	float acc = 2.3f;
	
	World::ParticleSystem * plasmaGunPS = new World::ParticleSystem();
	plasmaGunPS->setName("plasmaGunPS");
	plasmaGunPS->init(bulletTexture);
	plasmaGunPS->setSizeRange(vec2(1.4, 1.4));
	plasmaGunPS->setEnergyRange(vec2(3, 3));
	plasmaGunPS->setEmissionRange(vec2(acc/0.291f, acc/0.291f));
	plasmaGunPS->setStartVelocity(vec3(0, 12, 0));
	plasmaGunPS->setStartRndVelocity(vec3(0, 0, 0));
	plasmaGunPS->setEmissionElipsoid(vec3(0, 0, 0));
	plasmaGunPS->setSizeGrow(1.0f);
	plasmaGunPS->setAnimColor(0, tuple4ub(255,255,255,255));
	plasmaGunPS->setRotation(true);
	plasmaGunPS->setStartRotation(360.0f);
	plasmaGunPS->setStartRotation(-720.0f);
	plasmaGunPS->setEmit(false);
	mSceneObject->addSceneObject(plasmaGunPS);
	
	World::SoundSource * plasmaGunSnd = new World::SoundSource;
	plasmaGunSnd->setName("plasmaGunSnd");
	plasmaGunSnd->setPlayAutomatically(false);
	plasmaGunSnd->setLoop(true);
	plasmaGunSnd->setSound(bulletSound);
	plasmaGunSnd->setPitch(acc);
	plasmaGunSnd->setRefRadius(1.0f);
	plasmaGunSnd->setMaxRadius(100.0f);
	mSceneObject->addSceneObject(plasmaGunSnd);
	
	mGunParticles = plasmaGunPS;
	mGunSound = plasmaGunSnd;
	
}