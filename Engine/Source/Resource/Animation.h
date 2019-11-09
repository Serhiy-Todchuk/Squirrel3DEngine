#pragma once

#include "AnimationTrack.h"
#include "ResourceStorage.h"
#include <map>

#ifdef	_WIN32
//	disable warning on extern before template instantiation
#	pragma warning( disable: 4231 )
#endif

namespace Squirrel {

namespace Resource { 

class Animatable;

//represents animation resource, controls animation tracks, 
//instance of this class does not change itself and it's content when using it, so it could be reused by many AnimationNode instances
class SQRESOURCE_API Animation
{
public:

	struct SubAnim
	{
		SubAnim(): mName(""), mRepeats(1)
		{
			mTimeRange.zero();
		}
		std::string		mName;
		int				mRepeats;
		Math::vec2		mTimeRange;
	};
	
	struct Channel
	{
		Channel(): mTargetId(-1), mTrack(NULL) {}
		Channel(_ID trgId, AnimationTrack * track): mTargetId(trgId), mTrack(track) {}
		_ID					mTargetId;
		AnimationTrack *	mTrack;
	};

	typedef std::vector<Channel>				TRACKS_MAP;
	typedef std::map<std::string, SubAnim>		SUBANIMS_MAP;

protected:

	std::string				mName;

	TRACKS_MAP				mTracksMap;
	Math::vec2				mTimeRange;
	SUBANIMS_MAP			mSubAnims;

	int						mFramesNum;

public:

	Animation(void);
	virtual ~Animation(void);

	void			setFramesNum(int fn)	{ mFramesNum = fn; }

	int				getFramesNum()	{ return mFramesNum; }
	TRACKS_MAP *	getTracks()		{ return &mTracksMap; }
	SUBANIMS_MAP *	getSubAnims()	{ return &mSubAnims; }
	Math::vec2		getTimeRange()	{ return mTimeRange;}
	void addSubAnim(const SubAnim& subAnim)
	{
		mSubAnims[subAnim.mName] = subAnim;
	}
	SubAnim& newSubAnim(const std::string& name)
	{
		return mSubAnims[name] = SubAnim();
	}

	void calcTimeRange();
	void addTrack(_ID targetComponentId, AnimationTrack * track);

	bool updateTarget(Animatable * target, float time);

	const std::string&	getName	()	const		{return mName;}
	void	setName	(const std::string& name)	{mName	= name;}

private:
	void addTrackToTimeRange(AnimationTrack * track);

};

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::vector<Animation::Channel>;
SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::map<std::string, Animation::SubAnim>;
#endif
	
}//namespace Resource { 

}//namespace Squirrel {