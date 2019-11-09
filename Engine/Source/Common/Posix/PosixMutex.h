#pragma once

#include "../Mutex.h"
#include <pthread.h>

namespace Squirrel {

class PosixMutex: public Mutex
{
	pthread_mutex_t mMutex;
public:
	PosixMutex();
	virtual ~PosixMutex();

	void lock();
	void unlock();
	bool tryLock();
};

}//namespace Squirrel {