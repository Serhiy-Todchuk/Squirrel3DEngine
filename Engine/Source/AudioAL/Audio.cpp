#include "Audio.h"
#include "Buffer.h"
#include "Source.h"
#include "Utils.h"
#include <Common/Log.h>

#ifdef _WIN32
#	include <al/al.h>
#	include <al/alc.h>
#elif __APPLE__
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#endif

namespace Squirrel {
namespace AudioAL { 

ALCdevice * mDevice = NULL;
ALCcontext * mContext = NULL;

bool CheckALCError(const char_t * location)
{
	ALenum errCode = alcGetError(mDevice);
	if(errCode != AL_NO_ERROR)
	{
		const char_t * str = (const char_t *)alcGetString(mDevice, errCode);
		Log::Instance().streamError("CheckALCError") << location << " " << str;
		Log::Instance().flush();
		//MessageBox(NULL, str, location, MB_OK);
		return true;
	}
	return false;
};

Audio::Audio()
{
	mDevice = NULL;
	mContext = NULL;
}

Audio::~Audio()
{
	if(mDevice != NULL)
	{
		if(mContext != NULL)
		{
			alcMakeContextCurrent(NULL);
			alcDestroyContext(mContext);
		}
		alcCloseDevice(mDevice);
	}
}

bool Audio::init()
{
	// Initialization
	mDevice = alcOpenDevice(NULL); // select the "preferred device"
	CheckALCError("OpenAL::Audio::init (alcOpenDevice)");

	if (mDevice != NULL) {

		const char_t * deviceName = (const char_t *)alcGetString(mDevice, ALC_DEVICE_SPECIFIER);

		int minorVersion = 0, majorVersion = 1;
		alcGetIntegerv(mDevice, ALC_MAJOR_VERSION, 1, &majorVersion);
		alcGetIntegerv(mDevice, ALC_MINOR_VERSION, 1, &minorVersion);

		Log::Instance().report("OpenAL: Initialized audio device", deviceName, Log::sevImportantMessage);

		Log::Instance().stream("OpenAL version supported ", Log::sevMessage) << majorVersion << "." << minorVersion;

		mContext = alcCreateContext(mDevice,NULL);
		CheckALCError("OpenAL::Audio::init (alcCreateContext)");

		if(mContext != NULL)
		{
			alcMakeContextCurrent(mContext);
			CheckALCError("OpenAL::Audio::init (alcMakeContextCurrent)");

			Log::Instance().stream("OpenAL: Audio context created successfully!", Log::sevMessage) << 
				"vendor: " << alGetString(AL_VENDOR) << " renderer: " << alGetString(AL_RENDERER) << " version: " << alGetString(AL_VERSION);
		}
	}

	return mContext != NULL;
}

void Audio::setListenerPosition(vec3 pos)
{
	alListenerfv(AL_POSITION, &pos.x);
}

void Audio::setListenerOrientation(vec3 dir, vec3 up)
{
	float ori[] = { dir.x, dir.y, dir.z, up.x, up.y, up.z };
	alListenerfv(AL_ORIENTATION, ori);
}

void Audio::setListenerVelocity(vec3 v)
{
	alListenerfv(AL_VELOCITY, &v.x);
}

void Audio::setMasterGain(float gain)
{
	alListenerf(AL_GAIN, gain);
}

void Audio::setDistanceModel(DISTANCE_MODEL model)
{
	ALenum alModel;
	switch(model)
	{
	case INVERSE:
		alModel = AL_INVERSE_DISTANCE;
		break;
	case LINEAR:
		//alModel = AL_LINEAR_DISTANCE;
		break;
	case EXPONENTIAL:
		//alModel = AL_EXPONENT_DISTANCE;
		break;
	}

	alDistanceModel(alModel);
}

void Audio::setDoplerFactor(float factor)
{
	alDopplerFactor(factor);
}

void Audio::setSpeedOfSound(float speed)
{
	//alSpeedOfSound(speed);
}

IBuffer * Audio::createBuffer()
{
	IBuffer * buffer = new Buffer();
	if(Utils::CheckALError("OpenAL: Failed to create buffer!"))
	{
		DELETE_PTR(buffer);
		return NULL;
	}
	return buffer;
}

ISource * Audio::createSource()
{
	ISource * source = new Source();
	if(Utils::CheckALError("OpenAL: Failed to create sound source!"))
	{
		DELETE_PTR(source);
		return NULL;
	}
	return source;
}

}//namespace AudioAL {
} //namespace Squirrel {
