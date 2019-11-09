#include "Mutex.h"
#if defined(_WIN32)
# include "Windows/WindowsMutex.h"
#else
# include "Posix/PosixMutex.h"
#endif

namespace Squirrel {

Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

Mutex * Mutex::Create()
{
#if defined(_WIN32)
	return new WindowsMutex();
#else
	return new PosixMutex();
#endif
}
	
MutexLock::MutexLock(Mutex * mutex): mMutex(mutex)
{
	if(mMutex)
		mMutex->lock();
}
	
MutexLock::~MutexLock()
{
	if(mMutex)
		mMutex->unlock();
}


}//namespace Squirrel {
