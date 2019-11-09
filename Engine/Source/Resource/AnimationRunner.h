#pragma once

#include "Animation.h"

#ifdef	_WIN32
//	disable warning on extern before template instantiation
#	pragma warning( disable: 4231 )
#endif

namespace Squirrel {

namespace Resource { 

//class represents animation instance and controls animation playing
//does not care about animation releasing
class SQRESOURCE_API AnimationNode
{
public:

	enum State
	{
		sStop = 0,
		sPlay,
		sPause
	};

public:
	AnimationNode();
	AnimationNode(Animation * anim);
	AnimationNode(Animation * anim, Animation::SubAnim * subAnim);
	virtual ~AnimationNode();

	//returns time left after stopping anim
	float update(float dtime);
	void play();
	void stop();
	void pause();

	//setters
	void	setName(const std::string& name) { mName = name; }
	void	setRepeatsNum(uint repeats)		{mRepeatsNum = repeats;}
	void	setTimeRange(Math::vec2 range)	{mTimeRange = range;}
	void	setTarget(Animatable * target)	{mTarget = target;}

	//getters
	Math::vec2			getTimeRange()	const { return mTimeRange; }
	int					getRepeatsNum()	const { return mRepeatsNum; }
	State				getState()		const { return mState; }
	bool				isPlaying()		const { return mState == sPlay; }
	const std::string&	getName()		const { return mName; }
	Animation *			getAnimation()	const { return mAnim; }
	Animatable *		getTarget()		const { return mTarget; }

private:

	std::string		mName;
	uint			mRepeatsNum;		//if 0 then infinite
	uint			mCurrRepeats;
	Animation *		mAnim;
	Animatable *	mTarget;
	State			mState;
	float			mCurrTime;
	float			mTimeSpeed;
	Math::vec2		mTimeRange;

private:

	void initMembers();
};

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::map<std::string, AnimationNode *>;
#endif
	
//
class SQRESOURCE_API AnimationRunner
{
public:

	typedef std::map<std::string, AnimationNode *> ANIMS_MAP;

private:

	ANIMS_MAP		mAnimations;
	AnimationNode *	mPlayingAnimation;
	std::string		mLastAnimName;

public:

	AnimationRunner();
	virtual ~AnimationRunner();

public:

	void update(float dtime);

	void addAnim(const std::string& name, Animation * anim);
	void addAnimNode(const std::string& name, AnimationNode * node);
	AnimationNode * getAnimNode(const std::string& name);
	bool playAnimNode(const std::string& name);
	void stop();
	bool isPlaying();

	ANIMS_MAP&	getAnimNodes()		{ return mAnimations; }
	std::string	getLastAnimName()	{ return mLastAnimName; }
};


}//namespace Resource { 

}//namespace Squirrel {