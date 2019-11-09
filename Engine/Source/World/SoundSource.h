#pragma once

#include <common/common.h>
#include <common/LookAtObject.h>
#include <Common/Data.h>
#include "SceneObject.h"
#include <Resource/Sound.h>
#include <Audio/ISource.h>

namespace Squirrel {
namespace World { 

using namespace Math;

class SQWORLD_API SoundSource: 
	public SceneObject
{
public:

	SoundSource();
	~SoundSource();

public:

	virtual void update(float dtime);

	bool isCone() const;

	//getters
	const Audio::ISource * getSource	(void) const	{return mEmitter;}
	Audio::ISource * getSource	(void)	{return mEmitter;}

	//setters
	bool		setSound(Resource::Sound * sound);
	bool		setSound(const std::string& snd);
	void		setMaxRadius(float a)		{ mMaxRadius	= a;}
	void		setRefRadius(float a)		{ mRefRadius	= a;}
	void		setLoop(bool a)				{ mLoop			= a;}
	void		setDirection(vec3 dir)		{ mDirection	= dir;}
	void		setPlayAutomatically(bool f){ mPlayAutomatically= f;}
	void		setPitch(float a)			{ mPitch	= a;}

	virtual void deserialize(Reflection::Deserializer * deserializer);

protected:

	virtual void calcAABB();

private:

	void onSoundNameChanged();

private:

	bool mPlayAutomatically;

	vec3 mDirection;

	Resource::Sound * mSound;

	Audio::ISource * mEmitter;

	vec3 mPrevPosition;

	float mPitch;

	float mGain;

	bool mLoop;

	float mMaxRadius;
	float mRefRadius;

	float mConeOuterAngle;
	float mConeInnerAngle;
	float mConeOuterGain;

	std::string mSoundName;
};


}//namespace World { 
}//namespace Squirrel {
