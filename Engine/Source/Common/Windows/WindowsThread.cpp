#include "WindowsThread.h"
#include "../Log.h"
#include <process.h>

namespace Squirrel {

WindowsThread::WindowsThread(Runnable * r)
	: Thread(r) 
{
	if(!mRunnable)
		printError("WindowsThread(std::auto_ptr<Runnable> r)", __FILE__, __LINE__);
	mThreadHandle = 
		(HANDLE)_beginthreadex(NULL,0,WindowsThread::StartThreadRunnable,
				(LPVOID)this, CREATE_SUSPENDED, &mThreadID);
	if(!mThreadHandle)
		printError("_beginthreadex",__FILE__, __LINE__);
}

unsigned WINAPI WindowsThread::StartThreadRunnable(LPVOID pVoid) {
	WindowsThread* runnableThread = static_cast<WindowsThread*>(pVoid);
	runnableThread->mResult = runnableThread->mRunnable->run();
	return reinterpret_cast<unsigned>(runnableThread->mResult);
}

WindowsThread::~WindowsThread() {
	if(mThreadID != GetCurrentThreadId()) {
		DWORD rc = CloseHandle(mThreadHandle);
		if(!rc) 
			printError("CloseHandle", __FILE__, __LINE__);
	}
}

void WindowsThread::start() {
	ASSERT(mThreadHandle);
	DWORD rc = ResumeThread(mThreadHandle);
	// thread created is in suspended state, 
	// so this starts it running
	if(!rc) 
		printError("ResumeThread", __FILE__, __LINE__);
}

bool WindowsThread::isFinished()
{
	ASSERT(mThreadHandle);
	DWORD ret = WaitForSingleObject(mThreadHandle, 0);
	return ret == WAIT_OBJECT_0;
}

void WindowsThread::printError(LPSTR lpszFunction, LPSTR fileName, int lineNumber)
{
	TCHAR szBuf[256];
	LPSTR lpErrorBuf = NULL;
	DWORD errorCode=GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER||
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&lpErrorBuf,
		0,
		NULL);

	wsprintf(szBuf,"%s failed at line %d in %s with error %d: %s", 
		     lpszFunction, lineNumber, fileName, errorCode, lpErrorBuf);

	Log::Instance().error("WindowsThread::printError", szBuf);

	if(lpErrorBuf)
		LocalFree(lpErrorBuf);
}

}//namespace Squirrel {
