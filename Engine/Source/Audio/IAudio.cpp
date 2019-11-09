#include "IAudio.h"

namespace Squirrel {
namespace Audio { 

IAudio * IAudio::sActiveAudio = NULL;

IAudio::IAudio()
{
}

IAudio::~IAudio() 
{
	/*
	//go through the copy of pool so objects will be able to unregister itself from actual pool
	IContextObject::OBJECTS_POOL pool(mContextObjects);

	for(IContextObject::OBJECTS_POOL::iterator it = pool.begin(); it != pool.end(); ++it)
	{
		(*it)->setPool(NULL);
	}
	*/
}

}//namespace Audio {
} //namespace Squirrel {
