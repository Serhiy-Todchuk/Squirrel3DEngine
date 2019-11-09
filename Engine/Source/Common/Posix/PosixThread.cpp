#include "PosixThread.h"
#include "../Log.h"

#ifdef __APPLE__
# include "../Mac/MacUtils.h"
#endif

namespace Squirrel {

PosixThread::PosixThread(Runnable * r) :
		Thread(r), completed(false) {
	if(!mRunnable) {
		Log::Instance().streamError("PosixThread::PosixThread(auto_ptr<Runnable> r, bool isDetached)") << "Thread::Thread(auto_ptr<Runnable> r, bool isDetached)"\
		"failed at " << " " << __FILE__ <<":" << __LINE__ << "-" <<
		" runnable is NULL" << endl;
	}
}

void* PosixThread::StartThreadRunnable(void* pVoid) {
	// thread start function when a Runnable is involved
	
#ifdef __APPLE__
	void * autoreleasePool = MacUtils::OpenAutoreleasePool();
#endif
	
	PosixThread* runnableThread = static_cast<PosixThread*>(pVoid);
	ASSERT(runnableThread);
	runnableThread->result = runnableThread->mRunnable->run();
	runnableThread->completed = true;
	
#ifdef __APPLE__
	MacUtils::CloseAutoreleasePool(autoreleasePool);
#endif
	
	return runnableThread->result;
}
	
/*
void* PosixThread::StartThread(void* pVoid) {
	// thread start function when no Runnable is involved
	PosixThread* aThread = static_cast<PosixThread*>(pVoid);
	assert(aThread);
	aThread->start();
	aThread->completed = true;
	return aThread->result;
}
*/
	
PosixThread::~PosixThread() {}

void PosixThread::start() {
	
	completed = false;
	
	// initialize attribute object
	int status = pthread_attr_init(&threadAttribute);
	if(status) {
		printError("pthread_attr_init failed at", status, __FILE__, __LINE__);
	}

	// set the scheduling scope attribute
	status = pthread_attr_setscope(&threadAttribute,
					PTHREAD_SCOPE_SYSTEM);
	if(status) {
		printError("pthread_attr_setscope failed at", status, __FILE__, __LINE__);
	}

	if(!detached) {
/*		if(!mRunnable) {
			status = pthread_create(&PthreadThreadID, &threadAttribute,
				PosixThread::StartThread, (void*)this);	
			if(status) {
				printError("pthread_create failed at", status, __FILE__, __LINE__);
			}
		}
		else */{
			status = pthread_create(&PthreadThreadID, &threadAttribute,
				PosixThread::StartThreadRunnable, (void*)this);	
			if(status) {
				printError("pthread_create failed at", status, __FILE__, __LINE__);
			}
		}
	}
	else {
		// set the detachstate attribute to detached
		status = pthread_attr_setdetachstate(&threadAttribute,
						PTHREAD_CREATE_DETACHED);	
		if(status) {
			printError("pthread_attr_setdetachstate failed at", status, __FILE__, __LINE__);
		}
/*
		if(!mRunnable) {
			status = pthread_create(&PthreadThreadID, &threadAttribute,
				PosixThread::StartThread, (void*)this);	
			if(status) {
				printError("pthread_create failed at", status,
					__FILE__, __LINE__);
				exit(status);
			}
		}
		else*/ {
			status = pthread_create(&PthreadThreadID, &threadAttribute,
				PosixThread::StartThreadRunnable, (void*)this);	
			if(status) {
				printError("pthread_create failed at", status, __FILE__, __LINE__);
			}
		}
	}
	status = pthread_attr_destroy(&threadAttribute);
	if(status) {
		printError("pthread_attr_destroy failed at", status, __FILE__, __LINE__);
	}
}
    
    
bool PosixThread::isFinished()
{
    return completed;
}

void PosixThread::printError(const char * msg, int status, const char* fileName, int lineNumber) {
	Log::Instance().streamError("Thread::printError") << msg << " " << fileName << ":" << lineNumber <<
		"-" << strerror(status) << endl;
}

}//namespace Squirrel {
