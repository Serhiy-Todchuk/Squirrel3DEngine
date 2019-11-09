#pragma once

#include <Audio/ISource.h>
#include <common/types.h>
#include "macros.h"

namespace Squirrel {

namespace AudioAL { 

using namespace Math;
using namespace Audio;

class SQOPENAL_API Source:
	public ISource
{
public://ctor/destr

	Source();
	virtual ~Source();

public: //methods

	virtual void setPosition(vec3 pos);
	virtual void setDirection(vec3 dir);
	virtual void setVelocity(vec3 v);

	virtual void setPitch(float pitch);
	virtual void setGain(float gain);

	virtual void setLoop(bool loop);

	virtual void setRadius(float maxRadius, float halfRadius);
	virtual void setCone(float outerAngle, float innerAngle, float outerGain);

	virtual void play();
	virtual void stop();
	virtual void pause();
	virtual void offset(float seocnds);

	virtual void attachBuffer(IBuffer * buffer);

	virtual STATE getState(); 

private:

	uint mId;
};


} //namespace AudioAL {

} //namespace Squirrel {
