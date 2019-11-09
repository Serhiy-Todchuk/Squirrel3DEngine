#pragma once

#include <Common/types.h>
#include <Common/IDMap.h>
#include <Common/Data.h>
#include <map>
#include "macros.h"
#include "Animation.h"

#ifdef	_WIN32
//	disable warning on extern before template instantiation
#	pragma warning( disable: 4231 )
#endif

namespace Squirrel {

namespace Resource { 

class AnimationRunner;
//class Animation;

class SQRESOURCE_API AnimatableResource
{
public:

	enum TARGET_COMPONENTS
	{
		ANIM_TRANSLATE_X	= 1,
		ANIM_TRANSLATE_Y	= 2,
		ANIM_TRANSLATE_Z	= 4,
		ANIM_TRANSLATE_XYZ	= 7,
		ANIM_ROTATE_X		= 8,
		ANIM_ROTATE_Y		= 16,
		ANIM_ROTATE_Z		= 32,
		ANIM_ROTATE_XYZ		= 56,
		ANIM_SCALE_X		= 64,
		ANIM_SCALE_Y		= 128,
		ANIM_SCALE_Z		= 256,
		ANIM_SCALE_XYZ		= 448,
		ANIM_MATRIX4X4		= 512,
		ANIM_MATRIX3X3		= 1024,
		ANIM_QUATERNION		= 2048,
	};

	enum TARGET_TYPE
	{
		TRG_UNKNOWN = 0,
		TRG_MODEL_NODE = 1,
		TRG_SKELETON_BONE,
		TRG_OTHER
	};

	typedef IDMap<int>							TARGETS_MAP;
	typedef std::map<std::string, Animation *>	ANIMS_MAP;

protected:
	TARGETS_MAP		mTargetsMap;
	TARGET_TYPE		mTargetType;
	ANIMS_MAP		mAnimations;

public:
	AnimatableResource();
	virtual ~AnimatableResource();

	_ID addTarget(int target)
	{
		return mTargetsMap.add(target);
	}

	const ANIMS_MAP& getAnimations() const { return mAnimations; }
	Animation * getAnimation(const std::string& name);
	Animation * createAnimation(const std::string& name);

	TARGET_TYPE	getTargetType() const { return mTargetType; }

	void fillAnimRunner(AnimationRunner * animRunner);

protected:

	Animation * loadAnim(Data * data);
	bool saveAnim(Data * data, Animation * anim);

	bool loadAnims(Data * data);
	bool saveAnims(Data * data);
};

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API std::map<std::string, Animation *>;
#endif 
	
}//namespace Resource { 

}//namespace Squirrel {