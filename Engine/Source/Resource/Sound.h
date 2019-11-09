#pragma once

#include <Common/types.h>
#include "ResourceStorage.h"
#include "SoundLoader.h"
#include <Audio/IBuffer.h>

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API Sound:
	public StoredObject
{
	Audio::IBuffer * mBuffer;
	SoundLoader * mLoader;
	Data * mData;

public:
	Sound(void);
	Sound(Audio::IBuffer * buffer);
	Sound(Data * sndData, SoundLoader * loader);
	virtual ~Sound(void);

	Audio::IBuffer *	getBuffer(void)		{ return mBuffer; }

	Audio::IBuffer *	loadAll();

	Audio::IBuffer *	loadStreamed(double offset, double duration);
};


}//namespace Resource { 

}//namespace Squirrel {