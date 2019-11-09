#pragma once

#include "../Thread.h"
#include <pthread.h>

namespace Squirrel {

using namespace std;

class PosixThread:
	public Thread
{
public:
	PosixThread(Runnable * run);
	PosixThread();
	virtual ~PosixThread();

	virtual bool isFinished();
	virtual void start();

private:

	// thread ID
	pthread_t PthreadThreadID;
	// true if thread created in detached state
	bool detached;
	pthread_attr_t threadAttribute;

	volatile bool completed;
	
	// stores return value from run()
	void* result;
	static void* StartThreadRunnable(void* pVoid);
	static void* StartThread(void* pVoid);
	void printError(const char * msg, int status, const char* fileName, int lineNumber);
};

}//namespace Squirrel {