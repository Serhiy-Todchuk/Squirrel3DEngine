// DynamicLibrary.cpp: implementation of the DynamicLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "DynamicLibrary.h"

#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include <windows.h>
#else
# include <dlfcn.h>
#endif

namespace Squirrel {

const char_t * DynamicLibrary::GetDLExtension()
{
#if defined(_WIN32)
	return "dll";
#elif defined(__APPLE__)
	return "dylib";
#else
	return "so";
#endif
}

DynamicLibrary::DynamicLibrary():
	mHandle(NULL)
{
}

DynamicLibrary::DynamicLibrary(const char_t * dlfileName):
	mHandle(NULL)
{
	open(dlfileName);
}

DynamicLibrary::~DynamicLibrary()
{
	close();
}

bool DynamicLibrary::open(const char_t * dlfileName)
{
	if(isOpened())
		return false;
	
#ifdef _WIN32
	mHandle = (void *)LoadLibrary(dlfileName);
#else
	mHandle = dlopen(dlfileName, RTLD_LAZY);
#endif

	return mHandle != NULL;
}

void DynamicLibrary::close()
{
	if(!isOpened())
		return;

#ifdef _WIN32
	FreeLibrary((HMODULE)mHandle);
#else
	dlclose(mHandle);
#endif
}

void * DynamicLibrary::getSymbol(const char_t * name)
{
#ifdef _WIN32
	return (void *)GetProcAddress((HMODULE)mHandle, name);
#else
	return dlsym(mHandle, name);
#endif
}

bool DynamicLibrary::isOpened()
{
	return mHandle != NULL;
}

} //namespace Squirrel {