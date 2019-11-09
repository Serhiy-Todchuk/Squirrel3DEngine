#pragma once

#include "../common/macros.h"
#include "IBuffer.h"
#include <Math/vec3.h>

namespace Squirrel {

namespace Audio { 

using namespace Math;

class SQCOMMON_API ISource
{
public://nested types

	//corresponds to OpenAL state
	enum STATE
	{
		INITIAL = 0x1011,
		PLAYING,
		PAUSED,
		STOPPED,
	};

public://ctor/destr

	ISource() {};
	virtual ~ISource() {};

public: //methods

	virtual void setPosition(vec3 pos) = 0;
	virtual void setDirection(vec3 dir) = 0;
	virtual void setVelocity(vec3 v) = 0;

	virtual void setPitch(float pitch) = 0;
	virtual void setGain(float gain) = 0;

	virtual void setLoop(bool loop) = 0;

	virtual void setRadius(float maxRadius, float halfRadius) = 0;
	virtual void setCone(float outerAngle, float innerAngle, float outerGain) = 0;

	virtual void play() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void offset(float seocnds) = 0;

	virtual void attachBuffer(IBuffer * buffer) = 0;

	virtual STATE getState() = 0; 
};


} //namespace Render {

} //namespace Squirrel {
