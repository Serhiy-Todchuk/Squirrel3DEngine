#include "AnimationRunner.h"
#include <Common/macros.h>

namespace Squirrel {

namespace Resource { 

//
// Animation node methods
//

AnimationNode::AnimationNode() 
{
	initMembers();
}

AnimationNode::AnimationNode(Animation * anim) 
{
	initMembers();
	mAnim = anim;
	mTimeRange = anim->getTimeRange();
}

AnimationNode::AnimationNode(Animation * anim, Animation::SubAnim * subAnim)
{
	initMembers();
	mAnim = anim;
	mTimeRange = subAnim->mTimeRange;
	mRepeatsNum = subAnim->mRepeats;
}

AnimationNode::~AnimationNode() 
{
}

void AnimationNode::initMembers()
{
	mAnim		= NULL;
	mState		= sStop;
	mCurrTime	= 0;
	mTimeSpeed	= 1;
	mTimeRange	= Math::vec2(0, 2);
	mRepeatsNum = 0;
	mCurrRepeats= 0;
}

//returns time left after stopping anim
float AnimationNode::update(float dtime)
{
	float exceedTime = 0;
	float animTime = mTimeRange.y - mTimeRange.x;

	if(mState == sPlay)
	{
		mCurrTime += dtime;

		if(mCurrTime >= mTimeRange.y)
		{
			mCurrTime -= animTime;
			++mCurrRepeats;
		}
		/*deprecated
		//set appropriate target
		if(mTarget!=NULL && mAnim->getTarget()!=mTarget)
		{
			mAnim->setTarget(mTarget);
		}
		*/
		//repeat count is not infinite and it will be the last repeat
		if(mRepeatsNum > 0 && mCurrRepeats == mRepeatsNum)
		{
			exceedTime = mCurrTime - mTimeRange.x;
			//clamp repeat
			mAnim->updateTarget(mTarget, mTimeRange.y);
			stop();
		}
		else
		{
			mAnim->updateTarget(mTarget, mCurrTime);
		}
	}

	return exceedTime;
}

void AnimationNode::play()
{
	mState = sPlay;
	mCurrTime = mTimeRange.x;
}

void AnimationNode::stop()
{
	mState		= sStop;
	mCurrTime	= 0;
	mCurrRepeats= 0;
}

void AnimationNode::pause()
{
	mState = sPause;
}

//
// AnimationRunner methods
//

AnimationRunner::AnimationRunner() 
{
	mLastAnimName = "";
	mPlayingAnimation = NULL;
}

AnimationRunner::~AnimationRunner() 
{
	ANIMS_MAP::iterator it = mAnimations.begin();
	for(; it != mAnimations.end(); ++it)
	{
		DELETE_PTR(it->second);
	}
}

void AnimationRunner::addAnim(const std::string& name, Animation * anim)
{
	AnimationNode * node = new AnimationNode(anim);
	addAnimNode(name, node);
}

void AnimationRunner::addAnimNode(const std::string& name, AnimationNode * node)
{
	node->setName(name);
	mAnimations[name] = node;
}

AnimationNode * AnimationRunner::getAnimNode(const std::string& name)
{
	ANIMS_MAP::iterator it = mAnimations.find(name);
	return it != mAnimations.end() ? it->second : NULL;
}

bool AnimationRunner::playAnimNode(const std::string& name)
{
	AnimationNode * anim = getAnimNode(name);
	if(anim)
	{
		stop();
		anim->play();
		mPlayingAnimation = anim;
		mLastAnimName = name;
		return true;
	}
	return false;
}

void AnimationRunner::stop()
{
	if(mPlayingAnimation != NULL)
	{
		mPlayingAnimation->stop();
		//mPlayingAnimation = NULL;
	}
}

bool AnimationRunner::isPlaying()
{
	return mPlayingAnimation != NULL;
}

void AnimationRunner::update(float dtime)
{
	if(mPlayingAnimation != NULL)
	{
		switch(mPlayingAnimation->getState())
		{
		case AnimationNode::sPlay:
		case AnimationNode::sPause:
			mPlayingAnimation->update(dtime);
			break;
		case AnimationNode::sStop:
			mPlayingAnimation = NULL;
		}
	}

	//for now support only one playing animation
	/*
	ANIMS_MAP::iterator it;
	for(it = getAnimNodes().begin(); it != getAnimNodes().end(); ++it)
	{
		it->second->update(dtime);
	}
	*/
}


}//namespace Resource { 

}//namespace Squirrel {