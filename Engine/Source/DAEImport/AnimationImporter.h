#pragma once

#include <Resource/ResourceManager.h>
#include <Resource/AnimationTrack.h>
#include <Resource/AnimatableResource.h>
#include <Resource/Animation.h>
#include "BaseImporter.h"

namespace Squirrel {
namespace DAEImport { 

using Resource::AnimationTrack;

class SQDAEIMPORTER_API AnimationImporter: public BaseImporter
{
public:

	struct AnimChannel
	{
		UniqueId			targetId;
		UniqueId			targetSubId;
		int					targetMember;
		AnimationTrack	  * animTrack;
	};

	typedef std::list<AnimChannel>		CHANNELS_LIST;

private:

	/** map: id of sampler <-> AnimationTrack */
	std::map<UniqueId, AnimationTrack *>	mAnimTracksMap;

 	/** map: id of sampler <-> AnimChannel */
	std::map<UniqueId, AnimChannel>			mAnimChannelsMap;

	std::map<UniqueId, CHANNELS_LIST>		mChannelsByTargetId;

public:

	AnimationImporter(void);
	virtual ~AnimationImporter(void);

	virtual void importObject(pugi::xml_node node);

	const std::map<UniqueId, CHANNELS_LIST>& sortChannelsByTargetId();

	const CHANNELS_LIST * getChannelsListForNode(const UniqueId& nodeId) const;
	//const std::map<UniqueId, AnimChannel>&	getAnimChannelsMap() const { return mAnimChannelsMap; }
	//const AnimChannel * getAnimChannel(const UniqueId& fwAnimId) const;
};

}//namespace DAEImport { 
}//namespace Squirrel {