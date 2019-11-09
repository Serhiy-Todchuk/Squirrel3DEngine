#pragma once

#include "macros.h"
#include "types.h"

namespace Squirrel {

class SQCOMMON_API Mutex
{
private:
	Mutex(const Mutex&);
	const Mutex& operator=(const Mutex&);

public:
	Mutex();
	virtual ~Mutex();

	virtual void lock() = 0;
	virtual void unlock() = 0;
	
	virtual bool tryLock() = 0;
	
	Mutex * Create();
};
	
class SQCOMMON_API MutexLock
{
	Mutex * mMutex;
	
public:
	MutexLock(Mutex * mutex);
	~MutexLock();
};

}//namespace Squirrel {