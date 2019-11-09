#pragma once

#include "ResourceStorage.h"
#include "SoundLoader.h"
#include "Sound.h"

namespace Squirrel {
namespace Resource { 

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API ResourceStorage<Sound>;
#endif
	
//TODO: create base class Library (analog of singleton template impl)
class SQRESOURCE_API SoundStorage: 
	public ResourceStorage<Sound>
{
	static SoundStorage * sActiveLibrary;

	class SoundLoaderCreator
	{
	public:
		SoundLoaderCreator(const std::string& tag_): tag(tag_) {}
		std::string tag;
		virtual SoundLoader * create() = 0;
	};

	template <class _TSoundLoader>
	class SoundLoaderCreatorImpl:
		public SoundLoaderCreator
	{
	public:
		SoundLoaderCreatorImpl(const std::string& tag_): SoundLoaderCreator(tag_) {}
		virtual SoundLoader * create()
		{
			return new _TSoundLoader;
		}
	};

public:
	SoundStorage();
	virtual ~SoundStorage();

	void setAsActive();
	static SoundStorage * Active();

protected:

	virtual Sound* load(Data * data);
	virtual bool save(Sound* resource, Data * data, std::string& fileName);

private:

	std::list<SoundLoaderCreator *> mLoaderCreators;
};


}//namespace Resource {
}//namespace Squirrel {