// SoundSource.cpp: implementation of the SoundSource class.
//
//////////////////////////////////////////////////////////////////////

#include "SoundSource.h"
#include <Resource/SoundStorage.h>
#include <Audio/IAudio.h>

namespace Squirrel {
namespace World { 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SQREFL_REGISTER_CLASS_SEED(World::SoundSource, WorldSoundSource);

SoundSource::SoundSource():
	mSound(NULL), mEmitter(NULL), mPrevPosition(0, 0, 0), mPlayAutomatically(false),
	mPitch(1.0f), mGain(1.0f), mLoop(false), mMaxRadius(100), mRefRadius(1), 
	mConeOuterAngle(360), mConeInnerAngle(360), mConeOuterGain(1), mDirection(0, 0, 0)
{
	SQREFL_SET_CLASS(World::SoundSource);

	wrapAtomicField("PlayAutomatically",&mPlayAutomatically);
	wrapAtomicField("Pitch",			&mPitch);
	wrapAtomicField("Gain",				&mGain);
	wrapAtomicField("Loop",				&mLoop);
	wrapAtomicField("MaxRadius",		&mMaxRadius);
	wrapAtomicField("RefRadius",		&mRefRadius);
	wrapAtomicField("ConeOuterAngle",	&mConeOuterAngle);
	wrapAtomicField("ConeInnerAngle",	&mConeInnerAngle);
	wrapAtomicField("ConeOuterGain",	&mConeOuterGain);
	wrapAtomicField("ConeDirection",	&mDirection.x, 3);

	Reflection::Object::Field * field = wrapAtomicField("SoundName",		&mSoundName);
	field->setChangeHandler(this, &SoundSource::onSoundNameChanged);

	mEmitter = Audio::IAudio::GetActive()->createSource();
}

SoundSource::~SoundSource()
{
	if(mEmitter != NULL)
	{
		DELETE_PTR(mEmitter);
	}
	if(mSound != NULL)
	{
		SoundStorage::Active()->release(mSound->getID());
	}
}

bool SoundSource::setSound(Resource::Sound * sound)
{
	if(mSound != NULL)
	{
		SoundStorage::Active()->release(mSound->getID());
	}

	mSound = sound;

	if(mSound != NULL)
	{
		Audio::IBuffer * buffer = mSound->loadAll();

		if(buffer == NULL)
			return false;

		mEmitter->attachBuffer( buffer );

		mSoundName = mSound->getName();

		if(mPlayAutomatically)
		{
			mEmitter->play();
		}
	}

	return true;
}

bool SoundSource::setSound(const std::string& snd)
{
	Resource::Sound * sound = SoundStorage::Active()->add(snd);

	if(!sound)
		return false;

	return setSound( sound );
}

void SoundSource::onSoundNameChanged()
{
	Resource::Sound * sound = SoundStorage::Active()->add(mSoundName);
	setSound( sound );
}

bool SoundSource::isCone() const
{
	return mConeInnerAngle < 360.0f && mConeOuterGain < 1.0f;
}

void SoundSource::deserialize(Reflection::Deserializer * deserializer)
{
	SceneObject::deserialize(deserializer);

	if(mSoundName.length() > 0)
	{
		setSound(mSoundName);
	}
}

void SoundSource::update(float dtime)
{
	if(mEmitter != NULL)
	{
		vec3 pos = getPosition();
		vec3 vel = pos - mPrevPosition;
		mPrevPosition = pos;

		mEmitter->setPosition( pos );
		mEmitter->setVelocity( vel );

		mEmitter->setPitch(mPitch);
		mEmitter->setGain(mGain);
		mEmitter->setLoop(mLoop);
		mEmitter->setRadius(mMaxRadius, mRefRadius);

		mEmitter->setCone(mConeOuterAngle, mConeInnerAngle, mConeOuterGain);

		if(isCone())
		{
			vec3 dir = getRotation().apply(mDirection);
			mEmitter->setDirection(dir);
		}
	}
}

void SoundSource::calcAABB()
{
	mAABB.setPoint(getPosition());
	mAABB.grow(mMaxRadius);
}

}//namespace World { 
}//namespace Squirrel {
