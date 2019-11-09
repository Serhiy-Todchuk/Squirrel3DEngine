#include "Source.h"
#include "Buffer.h"
#include "Utils.h"

#ifdef _WIN32
#	include <al/al.h>
#elif __APPLE__
#	include <OpenAL/al.h>
#endif

namespace Squirrel {
namespace AudioAL { 

Source::Source()
{
	alGenSources(1, &mId);
}

Source::~Source()
{
	if(alIsSource(mId))
		alDeleteSources(1, &mId); 
}

void Source::setPosition(vec3 pos)
{
	alSourcefv(mId, AL_POSITION, &pos.x);
}

void Source::setDirection(vec3 dir)
{
	alSourcefv(mId, AL_DIRECTION, &dir.x);
}

void Source::setVelocity(vec3 v)
{
	alSourcefv(mId, AL_VELOCITY, &v.x);
}

void Source::setPitch(float pitch)
{
	alSourcef(mId, AL_PITCH, pitch);
}

void Source::setGain(float gain)
{
	alSourcef(mId, AL_GAIN, gain);
}

void Source::setLoop(bool loop)
{
	alSourcei(mId, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void Source::setRadius(float maxRadius, float halfRadius)
{
	alSourcef(mId, AL_MAX_DISTANCE, maxRadius);
	alSourcef(mId, AL_REFERENCE_DISTANCE, halfRadius);
}

void Source::setCone(float outerAngle, float innerAngle, float outerGain)
{
	alSourcef(mId, AL_CONE_OUTER_ANGLE, outerAngle);
	alSourcef(mId, AL_CONE_INNER_ANGLE, innerAngle);
	alSourcef(mId, AL_CONE_OUTER_GAIN,	outerGain);
}

void Source::play()
{
	alSourcePlay(mId);
	CHECK_AL_ERROR;
}

void Source::stop()
{
	alSourceStop(mId);
}

void Source::pause()
{
	alSourcePause(mId);
}

void Source::offset(float seconds)
{
	//alSourcef(mId, AL_SEC_OFFSET, seconds);
}

void Source::attachBuffer(IBuffer * buffer)
{
	Buffer * alBuffer = static_cast<Buffer *>(buffer);
	alSourcei(mId, AL_BUFFER, alBuffer->getId());
	CHECK_AL_ERROR;
}

Source::STATE Source::getState()
{
	ALenum state = AL_INITIAL;

	alGetSourcei(mId, AL_SOURCE_STATE, &state);
	CHECK_AL_ERROR;

	return (Source::STATE)state;
}

} //namespace AudioAL {
} //namespace Squirrel {
