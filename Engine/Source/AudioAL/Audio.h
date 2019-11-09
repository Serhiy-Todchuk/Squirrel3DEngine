#pragma once

#include <Audio/IAudio.h>
#include <Common/types.h>
#include "macros.h"

namespace Squirrel {

namespace AudioAL { 

using namespace Math;
using namespace Audio;

class SQOPENAL_API Audio:
	public IAudio
{
public:

	Audio();
	virtual ~Audio();

	virtual bool init();

	virtual void setListenerPosition(vec3 pos);
	virtual void setListenerOrientation(vec3 dir, vec3 up);
	virtual void setListenerVelocity(vec3 v);

	virtual void setMasterGain(float gain);
	virtual void setDistanceModel(DISTANCE_MODEL model);
	virtual void setDoplerFactor(float factor);
	virtual void setSpeedOfSound(float speed);

	//create operations
	virtual IBuffer *	createBuffer();
	virtual ISource *	createSource();
};


}//namespace AudioAL { 

}//namespace Squirrel {

