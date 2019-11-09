#pragma once

#include "macros.h"
#include "types.h"
#include <memory>

namespace Squirrel {

class SQCOMMON_API Runnable 
{
public:
	virtual void* run() = 0;
	virtual ~Runnable();
};

class SQCOMMON_API Thread 
{
protected:

	Runnable * mRunnable;

private:
	Thread(const Thread&);
	const Thread& operator=(const Thread&);

public:
	Thread(Runnable * run);
	virtual ~Thread();

	virtual void start() = 0;
	virtual bool isFinished() = 0;

	static Thread * Create(Runnable * run);
};

}//namespace Squirrel {