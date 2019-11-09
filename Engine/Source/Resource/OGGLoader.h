#pragma once

#include "SoundLoader.h"

struct OggVorbis_File;

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API OGGLoader:
	public SoundLoader
{
	OggVorbis_File *mOVFile;

	char * mBuffer;
	uint32 mBufferSize;

	bool mIsSeekable;

public:
	OGGLoader();
	~OGGLoader();

	virtual bool open(Data * srcData);
	virtual bool load(Audio::IBuffer * targetBuffer, double offset, double duration);
	virtual void close();

	virtual bool isSeekable(Data * srcData) { return mIsSeekable; }
};


}//namespace Resource { 

}//namespace Squirrel {