// Platform.cpp: implementation of the Platform class.
//
//////////////////////////////////////////////////////////////////////

#include "Platform.h"

#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
# include "Windows/WindowsWindowManager.h"
#else
# include "Mac/MacUtils.h"
# include "Mac/MacWindowManager.h"
#endif

namespace Squirrel {

WindowManager * Platform::CreateWindowManager()
{
#ifdef _WIN32
	return new WindowsWindowManager();
#else
	return new MacWindowManager();
#endif
}
	
std::string Platform::GetAppName()
{
#ifdef _WIN32
	const DWORD strBufSize = 256;
	char_t strBuf[strBufSize];
	if(GetModuleFileName(NULL, strBuf, strBufSize) > 0)
	{
		return strBuf;
	}
	return "";
#else
	std::string str = MacUtils::GetBundleName();
	return str;
#endif
}

std::string Platform::GetRootPath()
{
#ifdef _WIN32
	char strBuf[ 255 ];
	strBuf[0] = '\0';
	GetCurrentDirectory( 255, strBuf );
	return strBuf;
#elif __APPLE__
	return MacUtils::GetBundleParentFolder();
#endif
}

void Platform::TerminateProcess()
{
#ifdef _WIN32
	PostQuitMessage(0);
#else
	MacUtils::TerminateProcess();
#endif		
}
	
void Platform::DebugLog(const char_t * str)
{
#ifdef _WIN32
	OutputDebugString(str);
#else
	MacUtils::DebugLog(str);
#endif
}
	
} //namespace Squirrel {