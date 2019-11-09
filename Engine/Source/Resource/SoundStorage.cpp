#include "SoundStorage.h"
#include "WAVLoader.h"
#include "OGGLoader.h"
#include <FileSystem/Path.h>
#include <Audio/IAudio.h>
#include <Common/Settings.h>
#include <algorithm>
#include <cctype>
#include <map>

namespace Squirrel {

namespace Resource { 

using namespace Audio;

SoundStorage * SoundStorage::sActiveLibrary = NULL;

SoundStorage * SoundStorage::Active()
{
	return sActiveLibrary;
}

SoundStorage::SoundStorage()
{
	mLoaderCreators.push_back(new SoundLoaderCreatorImpl<WAVLoader>("RIFF"));
	mLoaderCreators.push_back(new SoundLoaderCreatorImpl<OGGLoader>("OggS"));

	//mMapFiles = false;
}

SoundStorage::~SoundStorage()
{
	if(this == sActiveLibrary)
	{
		sActiveLibrary = NULL;
	}

	for(std::list<SoundLoaderCreator *>::iterator it = mLoaderCreators.begin(); 
		it != mLoaderCreators.end(); ++it)
	{
		DELETE_PTR(*it);
	}
}

void SoundStorage::setAsActive()
{
	sActiveLibrary = this;
}

Sound* SoundStorage::load(Data * data)
{
	ASSERT(data);

	//find appropriate loader creator
	SoundLoaderCreator * loaderCreator = NULL;
	for(std::list<SoundLoaderCreator *>::iterator it = mLoaderCreators.begin(); 
		it != mLoaderCreators.end(); ++it)
	{
		if(strncmp((char*)data->getData(), (*it)->tag.c_str(), 4) == 0)
		{
			loaderCreator = (*it);
			break;
		}
	}

	if(loaderCreator == NULL)
		return false;

	SoundLoader * loader = loaderCreator->create();

	//create sound resource
	Sound * sound = new Sound(data, loader);

	//Sound takes care about data and loader memory

	return sound;
}

bool SoundStorage::save(Sound* resource, Data * data, std::string& fileName)
{
	return false;
}


}//namespace Resource { 

}//namespace Squirrel {