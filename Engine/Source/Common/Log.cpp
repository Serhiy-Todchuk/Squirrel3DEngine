// Log.cpp: implementation of the Log class.
//
//////////////////////////////////////////////////////////////////////

#include "Log.h"
#include "TimeCounter.h"
#include "Platform.h"
#include <cstdio>

#ifdef	_WIN32
# pragma	warning (disable:4996)
#endif

namespace Squirrel {

int ShowMessageBox(const char * title, const char * content, int type)
{
	//#ifdef _WIN32
	//return ::MessageBox(NULL, title, content, type);
    return 0;
}

Log& Log::Instance()
{
	static Log instance;
	return instance;
}

Log::Log():
	mInitialised(false), mSeverity(sevInformation)
{
	mStartTicks	= TimeCounter::GetTicks();
}

Log::~Log()
{
}

void Log::finish()
{
	report(NULL, "END LOG_FILE", sevCriticalError);
	flush();
}

void Log::init(const char_t * fname, Severity maxSev)
{
	mFileName	= fname;
	mSeverity	= maxSev;
	mStartTicks	= TimeCounter::GetTicks();
	mInitialised = true;
	mMessages.clear();

	//create file
	FILE * file = NULL;
	file = fopen(fname, "w+");
	if(file)
	{
		fprintf(file,"%s\n\n", "BEGIN LOG_FILE");
		fclose(file);
	}
}

void Log::flush()
{
	if(mInitialised)
	{
		FILE * file = NULL;
		file = fopen(mFileName.c_str(), "a+");
		if(file)
		{
			fprintf(file,"%s", mMessages.str().c_str());
		
			fclose(file);
		}
	}

	Platform::DebugLog(mMessages.str().c_str());

	mMessages.str(std::string());
	mDummyStream.str(std::string());
}

void Log::report(const char_t * module, const char_t * _report, Severity sev)
{
	stream(module, sev) << _report;
}

void Log::error(const char_t * module, const char_t * _report)
{
	streamError(module) << _report;
	flush();
}

void Log::warning(const char_t * module, const char_t * _report)
{
	streamWarning(module) << _report;
}

std::ostream& Log::stream(Severity sev)
{
	return stream(NULL, sev);
}

std::ostream& Log::stream(const char_t * module, Severity sev)
{
	if(sev > mSeverity)
	{
		return mDummyStream;
	}

	uint32 ticks = TimeCounter::GetTicks() - mStartTicks;

	mMessages << std::endl << ticks << "\t\t" << (module != NULL ? module : "") << (module != NULL ? " -> " : "");

	return mMessages;
}

std::ostream& Log::streamError(const char_t * module)
{
	if(sevError > mSeverity)
	{
		return mDummyStream;
	}

	uint32 ticks = TimeCounter::GetTicks() - mStartTicks;

	mMessages << std::endl << ticks << "\tError: " << (module != NULL ? module : "") << (module != NULL ? " -> " : "");

	return mMessages;
}

std::ostream& Log::streamWarning(const char_t * module)
{
	if(sevWarning > mSeverity)
	{
		return mDummyStream;
	}

	uint32 ticks = TimeCounter::GetTicks() - mStartTicks;

	mMessages << std::endl << ticks << "\tWarning: " << (module != NULL ? module : "") << (module != NULL ? " -> " : "");

	return mMessages;
}

} //namespace Squirrel {