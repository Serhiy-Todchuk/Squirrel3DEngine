// Log.h: interface for the Log class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <vector>
#include "tuple.h"
#include "macros.h"
#include <sstream>

namespace Squirrel {

class SQCOMMON_API Log  
{
public:

	enum Severity
	{
		sevCriticalError = 0,
		sevError,
		sevWarning,
		sevImportantMessage,
		sevMessage,
		sevInformation,
		sevGarbage
	};

public:
	Log();
	~Log();

	static Log& Instance(); 

	Severity getSeverity();

	void setSeverity(Severity maxSev);

	void init(const char_t * fname, Severity maxSev = sevMessage);

	void flush();
	void finish();

	void report(const char_t * module, const char_t * report, Severity sev);
	void error(const char_t * module, const char_t * report);
	void warning(const char_t * module, const char_t * report);

	std::ostream& stream(Severity sev = sevMessage);
	std::ostream& stream(const char_t * module, Severity sev = sevMessage);
	std::ostream& streamError(const char_t * module = NULL);
	std::ostream& streamWarning(const char_t * module = NULL);

private:

	std::stringstream mMessages;
	std::stringstream mDummyStream;

	uint32 mStartTicks;

	std::string mFileName;

	Severity mSeverity;
	
	bool mInitialised;
};

} //namespace Squirrel {