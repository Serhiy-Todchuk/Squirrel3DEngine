#pragma once

#include "../common/macros.h"
#include "IBuffer.h"
#include "ISource.h"

namespace Squirrel {

namespace Audio { 

using namespace Math;

class SQCOMMON_API IAudio
{
	static IAudio * sActiveAudio;

protected:

	//IContextObject::OBJECTS_POOL mContextObjects;

public:

	enum DISTANCE_MODEL
	{
		INVERSE,
		LINEAR,
		EXPONENTIAL
	};

	static const int SPEED_OF_SOUND = 340;//meters per second

public:

	IAudio();
	virtual ~IAudio();

	static IAudio * GetActive() { return sActiveAudio; }
	void setAsActive() { sActiveAudio = this; }

	virtual bool init() = 0;

	virtual void setListenerPosition(vec3 pos) = 0;
	virtual void setListenerOrientation(vec3 dir, vec3 up) = 0;
	virtual void setListenerVelocity(vec3 v) = 0;

	virtual void setMasterGain(float gain) = 0;
	virtual void setDistanceModel(DISTANCE_MODEL model) = 0;
	virtual void setDoplerFactor(float factor) = 0;
	virtual void setSpeedOfSound(float speed) = 0;

	//create operations
	virtual IBuffer *					createBuffer() = 0;
	virtual ISource *					createSource() = 0;

	/* //TODO
	virtual IEffect *					createEffect() = 0;
	virtual IFilter *					createFilter() = 0;
	*/
};


}//namespace Render { 

}//namespace Squirrel {

