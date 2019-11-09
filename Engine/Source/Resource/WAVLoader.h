#pragma once

#include "SoundLoader.h"

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API WAVLoader:
	public SoundLoader
{
	uint32 mDataSize;
	uint32 mDataOffset;

public:
	WAVLoader();
	~WAVLoader();

	virtual bool open(Data * srcData);
	virtual bool load(Audio::IBuffer * targetBuffer, double offset, double duration);
	virtual void close();
};


}//namespace Resource { 

}//namespace Squirrel {