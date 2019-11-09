#pragma once

#include "../Mutex.h"
#include <windows.h>

namespace Squirrel {

class WindowsMutex: public Mutex
{
	CRITICAL_SECTION mCriticalSection;
public:
	WindowsMutex();
	virtual ~WindowsMutex();

	void lock();
	void unlock();
	bool tryLock();
};

}//namespace Squirrel {