#include "Thread.h"
#if defined(_WIN32)
# include "Windows/WindowsThread.h"
#else
# include "Posix/PosixThread.h"
#endif

namespace Squirrel {

Runnable::~Runnable()
{

}

Thread::Thread(Runnable * run):
	mRunnable(run) 
{
}

Thread::~Thread()
{

}

Thread * Thread::Create(Runnable * run)
{
#if defined(_WIN32)
	return new WindowsThread(run);
#else
	return new PosixThread(run);
#endif
}

}//namespace Squirrel {
