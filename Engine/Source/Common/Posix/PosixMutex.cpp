#include "PosixMutex.h"

namespace Squirrel {

PosixMutex::PosixMutex()
{
	pthread_mutex_init(&mMutex, NULL);
}

PosixMutex::~PosixMutex()
{
	pthread_mutex_destroy(&mMutex);
}

void PosixMutex::lock()
{
	pthread_mutex_lock(&mMutex);
}
	
bool PosixMutex::tryLock()
{
	return pthread_mutex_trylock(&mMutex) == 0;
}

void PosixMutex::unlock()
{
	pthread_mutex_unlock(&mMutex);
}

}//namespace Squirrel {
