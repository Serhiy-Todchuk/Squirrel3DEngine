// TimeCounter.cpp: implementation of the TimeCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "TimeCounter.h"

namespace Squirrel {

#ifdef _WIN32
#include <mmsystem.h>
#elif __APPLE__
#include <mach/mach_time.h>
#else
#include <sys/time.h>
#include <curses.h>
#endif
    
//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

uint32 TimeCounter::GetTicks( )
{
#ifdef WIN32
  // don't use GetTickCount anymore because it's not accurate enough (~16ms resolution)
  // don't use QueryPerformanceCounter anymore because it isn't guaranteed to be strictly increasing on some systems and thus requires "smoothing" code
  // use timeGetTime instead, which typically has a high resolution (5ms or more) but we request a lower resolution on startup
    return timeGetTime( );
#elif __APPLE__
  uint64 current = mach_absolute_time( );
  static mach_timebase_info_data_t info = { 0, 0 };
  // get timebase info
  if( info.denom == 0 )
    mach_timebase_info( &info );
  uint64 elapsednano = current * ( info.numer / info.denom );
  // convert ns to ms
  return elapsednano / 1e6;
#else
  uint32 ticks;
  struct timespec t;
  clock_gettime( CLOCK_MONOTONIC, &t );
  ticks = t.tv_sec * 1000;
  ticks += t.tv_nsec / 1000000;
  return ticks;
#endif
}

TimeCounter& TimeCounter::Instance()
{
	static TimeCounter instance;
	return instance;
}

TimeCounter::TimeCounter()
{
#ifdef WIN32
	timeBeginPeriod(1);
#endif
	mNewCount = mFrameCount = mLastCount = 0;
	mFramesPerSecond = 0;
	mDeltaTime = 0;
	mTime = 0;
}

TimeCounter::~TimeCounter()
{
#ifdef WIN32
	timeEndPeriod(1);
#endif
}

void TimeCounter::calcTime()
{
	static uint prevTick = GetTicks();
	uint currTick = GetTicks();
	uint diff = currTick - prevTick;
	prevTick = currTick;
	mDeltaTime = float(diff) / 1000.0f;
	if(mPause) mDeltaTime = 0.0f;
	mTime += mDeltaTime;

	mNewCount=(float)currTick;
	mFrameCount++;

	for(unsigned i=0; i<mNodes.size(); i++)	
	{
		mNodes[i].acc+= mNodes[i].time;
		//mNodes[i].ms += mNodes[i].time;
	}

	if((mNewCount-mLastCount)>updateInterval)
	{
		for(unsigned i=0; i<mNodes.size(); i++)
		{
			mNodes[i].ms=mNodes[i].acc;
			if(mFrameCount) mNodes[i].ms/=mFrameCount;
			//mNodes[i].ms = mNodes[i].acc;
			mNodes[i].acc=0;

		}
		mFramesPerSecond = mFrameCount*updateInterval/(mNewCount-mLastCount);
		mLastCount=mNewCount;
		mFrameCount=0;
	}
}

void TimeCounter::setNodeTimeBegin(unsigned node)		
{ 
	if(node<mNodes.size()) 
		mNodes[node].time = GetTicks();
}
void TimeCounter::setNodeTimeEnd(unsigned node)			
{ 
	if(node<mNodes.size())
	{
		int oldTicks = mNodes[node].time;
		int newTicks = GetTicks();
		mNodes[node].time = newTicks - oldTicks;
		//if(mNodes[node].time < 0)
			//newTicks = newTicks;
	}
}

}//namespace Squirrel {