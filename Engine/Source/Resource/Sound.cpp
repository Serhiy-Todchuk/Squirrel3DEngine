#include "Sound.h"
#include <Audio/IAudio.h>
#include <Common/Log.h>

namespace Squirrel {

namespace Resource { 

using namespace Audio;

Sound::Sound(void):
	mLoader(NULL), mData(NULL), mBuffer(NULL)
{
}

Sound::~Sound(void)
{
	if(mLoader != NULL)
	{
		mLoader->close();
		DELETE_PTR(mLoader);
	}
	DELETE_PTR(mData);
	DELETE_PTR(mBuffer);
}

Sound::Sound(Data * sndData, SoundLoader * loader):
	mLoader(loader), mData(sndData), mBuffer(NULL)
{
}

Audio::IBuffer * Sound::loadAll()
{
	if(mBuffer)
		return mBuffer;

	mBuffer = Audio::IAudio::GetActive()->createBuffer();

	if(!mBuffer)
		return NULL;

	if(!mLoader->isOpened())
	{
		if(!mLoader->open(mData))
		{
			DELETE_PTR(mBuffer);
		}
	}

	if(mLoader->isOpened())
	{
		if(!mLoader->load(mBuffer, 0, 0))
		{
			DELETE_PTR(mBuffer);
		}
		mLoader->close();
	}

	DELETE_PTR(mLoader);
	DELETE_PTR(mData);

	return mBuffer;
}

Audio::IBuffer *	Sound::loadStreamed(double offset, double duration)
{
	Audio::IBuffer * buffer = Audio::IAudio::GetActive()->createBuffer();

	if(!buffer)
		return NULL;

	if(!mLoader->isOpened())
	{
		mLoader->open(mData);
	}

	if(!mLoader->load(buffer, offset, duration))
	{
		DELETE_PTR(buffer);
	}

	return buffer;
}

}//namespace Resource { 

}//namespace Squirrel {