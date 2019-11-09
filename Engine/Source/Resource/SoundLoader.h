#pragma once

#include <Common/Data.h>
#include <Audio/IBuffer.h>
#include "macros.h"

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API SoundLoader
{
protected:

	Data * mSrcData;

	uint32 mChannels;
	uint32 mFrequency;
	uint32 mBitsPerSample;

	double mDuration;

public:
	SoundLoader(): mSrcData(NULL) {}
	~SoundLoader() {}

	double getDuration() { return mDuration; }
	bool isOpened() { return mSrcData != NULL; }

	virtual bool open(Data * srcData) = 0;
	virtual bool load(Audio::IBuffer * targetBuffer, double offset, double duration) = 0;
	virtual void close() = 0;

	virtual bool isSeekable(Data * srcData) { return true; }
};


}//namespace Resource { 

}//namespace Squirrel {