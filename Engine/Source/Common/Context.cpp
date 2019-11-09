#include "Context.h"

namespace Squirrel {

const Caps& Context::getCaps() { 
	if(!mCapsInitialized)
	{
		mCapsInitialized = initCaps();
	}
	return mCaps;
}

}//namespace Squirrel {