// Platform.h: interface for the Platform class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "macros.h"
#include "WindowManager.h"

namespace Squirrel {

class SQCOMMON_API Platform
{
	Platform() {};
	~Platform() {};
public:

	static WindowManager * CreateWindowManager();
	static std::string GetAppName();
	static std::string GetRootPath();

	static void TerminateProcess();
	static void DebugLog(const char_t * str);
};

} //namespace Squirrel {