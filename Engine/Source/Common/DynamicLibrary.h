// DynamicLibrary.h: interface for the DynamicLibrary class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "macros.h"
#include "types.h"

namespace Squirrel {

class SQCOMMON_API DynamicLibrary  
{
public:
	DynamicLibrary();
	DynamicLibrary(const char_t * dlfileName);
	~DynamicLibrary();

	bool open(const char_t * dlfileName);
	void close();
	void * getSymbol(const char_t * name);
	bool isOpened();

	static const char_t * GetDLExtension();

private:
	void * mHandle;
};

} //namespace Squirrel {