#pragma once

#include "AnimatableResource.h"

namespace Squirrel {

namespace Resource { 

class Animation;

class SQRESOURCE_API Animatable
{
	friend class Animation;

protected:
	struct TargetInfo
	{
		int		mComponentsNum;
		float *	mTargetData;
		bool ** mChangeFlags;
	};

	typedef IDMap<TargetInfo *>	TARGETS_MAP;

protected:
	TARGETS_MAP				mTargetsMap;
	AnimatableResource *	mAnimResource;//TODO decide if need to remove it

public:
	Animatable();
	virtual ~Animatable();

	AnimatableResource *	getResource() const { return mAnimResource; }

	virtual void animComponentChanged(_ID targetId, int componentIndex) { };
};

}//namespace Resource { 

}//namespace Squirrel {

