// FPSWalker.h: interface for the FPSWalker class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <World/SceneObject.h>
#include <World/Behaviour.h>
#include <World/SoundSource.h>
#include <World/ParticleSystem.h>
#include <Render/Camera.h>
#include <common/tuple.h>

using namespace Squirrel;

class FPSWalker : public World::Behaviour
{
	Render::Camera * mCamera;

	float mSpeed;
	float mMouseSensivity;

	float mHeight;

	tuple2i mPrevMouse; 

	Math::vec3 mVelocity;

public:
	FPSWalker();
	virtual ~FPSWalker();

	World::SoundSource * mGunSound;
	World::ParticleSystem * mGunParticles;
	
	void initGun(const char_t * bulletTexture, const char_t * bulletSound);
	
protected:
	
	virtual void awake();
	virtual void update();
};

