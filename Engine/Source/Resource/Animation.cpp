#include "Animation.h"
#include "Animatable.h"
#include <Common/macros.h>

namespace Squirrel {

namespace Resource { 

Animation::Animation(void): 
	mFramesNum(0), mTimeRange(0, 0)
{
}

Animation::~Animation(void)
{
	TRACKS_MAP::iterator itTrack = mTracksMap.begin();
	for(; itTrack != mTracksMap.end(); ++itTrack)
	{
		DELETE_PTR(itTrack->mTrack);
	}
}

void Animation::addTrack(_ID targetComponentId, AnimationTrack * track)
{
	//if(mTracksMap.find(targetComponentId) == mTracksMap.end())
	{
		//force step anim
		if(	track->getInterpolationType() == AnimationTrack::LINEAR ||
			track->getInterpolationType() == AnimationTrack::STEP)
		{
			track->setInterpolationType(AnimationTrack::STEP);
		}
		else
		{
			track->convertToStep(24.0f);
		}

		mTracksMap.push_back(Channel(targetComponentId, track));
		addTrackToTimeRange(track);
	}
}

void Animation::calcTimeRange()
{
	const float bigTimeValue = 99999.0f;
	mTimeRange = Math::vec2(bigTimeValue, -bigTimeValue);
	TRACKS_MAP::iterator itTrack;
	for( itTrack  = getTracks()->begin();
		 itTrack != getTracks()->end();
		 ++itTrack )
	{
		addTrackToTimeRange( itTrack->mTrack );
	}
}

void Animation::addTrackToTimeRange(AnimationTrack * track)
{
	float frontTime	= track->getTimeline()->front();
	float backTime	= track->getTimeline()->back();
	if(mTimeRange.x > frontTime)	mTimeRange.x = frontTime;
	if(mTimeRange.y < backTime)		mTimeRange.y = backTime;
}

bool Animation::updateTarget(Animatable * targetsContainer, float time)
{
	ASSERT(targetsContainer!=NULL);

	TRACKS_MAP::iterator		itTrack;
	Animatable::TargetInfo *	targetInfo;
	_ID							tracksTargetId;
	AnimationTrack *			track;

	for(itTrack = mTracksMap.begin(); itTrack != mTracksMap.end(); ++itTrack)
	{
		tracksTargetId	= itTrack->mTargetId;
		track			= itTrack->mTrack;
		targetInfo		= targetsContainer->mTargetsMap.get(tracksTargetId);
		
		if(targetInfo != NULL)
		{
			if(targetInfo->mComponentsNum >= track->getTargetComponentsNum())
			{
				track->updateBuffer(time, targetInfo->mTargetData, targetInfo->mChangeFlags);
			}
			else
			{
				//TODO: err info
				return false;
			}
		}
		else
		{
			//TODO: err info
			return false;
		}
	}
	return true;
}

}//namespace Resource { 

}//namespace Squirrel {