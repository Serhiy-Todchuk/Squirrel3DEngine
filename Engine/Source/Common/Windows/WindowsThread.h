#pragma once

#ifdef _WIN32

#include "../Thread.h"
#include <windows.h>

namespace Squirrel {

class WindowsThread:
	public Thread
{
public:
	WindowsThread(Runnable * run);
	virtual ~WindowsThread();

	virtual void start();

	virtual bool isFinished();

private:
	HANDLE mThreadHandle;
	unsigned mThreadID;

	WindowsThread(const WindowsThread&);
	const WindowsThread& operator=(const WindowsThread&);

	// called when run() completes
	void setCompleted();
	// stores return value from run()
	void* mResult;

	void printError(LPTSTR lpszFunction, LPSTR fileName, int lineNumber);

	static unsigned WINAPI StartThreadRunnable(LPVOID pVoid);
};

}//namespace Squirrel {

#endif//_WIN32