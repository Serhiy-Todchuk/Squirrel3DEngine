#include "WindowsMutex.h"
#include <process.h>

namespace Squirrel {

WindowsMutex::WindowsMutex()
{
	InitializeCriticalSection(&mCriticalSection);
}

WindowsMutex::~WindowsMutex()
{
	DeleteCriticalSection(&mCriticalSection); 
}

void WindowsMutex::lock()
{
	EnterCriticalSection(&mCriticalSection);
}
	
bool WindowsMutex::tryLock()
{
	return TryEnterCriticalSection(&mCriticalSection) != 0;
}

void WindowsMutex::unlock()
{
	LeaveCriticalSection(&mCriticalSection);
}

}//namespace Squirrel {
