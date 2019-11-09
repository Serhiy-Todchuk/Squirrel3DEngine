#include "WindowManager.h"
#include "Log.h"

namespace Squirrel {

WindowManager * sActiveWindowManager = NULL;

WindowManager::WindowManager()
{
	mDefaultWinMode = Window::wmStandard;

	//ensure WindowManager has been created only once 
	if(sActiveWindowManager != NULL)
	{
		//TODO: throw exception
		Log::Instance().error("WindowManager", "WindowManager is creating second time! Ensure that it is created only once!");
	}
	sActiveWindowManager = this;
}

WindowManager::~WindowManager()
{
	for(std::vector<Window *>::iterator it = mWindows.begin(); it != mWindows.end(); ++it)
	{
		DELETE_PTR((*it));
	}
}

WindowManager * WindowManager::ActiveWindowManager()
{
	return sActiveWindowManager;
}

const std::vector<DisplaySettings>& WindowManager::getDisplayModes() 
{ 
	enumerateDisplayModes(); 
	return mDisplayModes; 
}

bool WindowManager::validateDisplayMode(DisplaySettings& ds)
{
	const std::vector<DisplaySettings>& modes = getDisplayModes();

	size_t index       = 0;
	int    refreshRate = 0;

	for(index = 0; index < modes.size(); index++)
		if(ds.colorBits == modes[index].colorBits &&
		   ds.height  == modes[index].height  &&
		   ds.width  == modes[index].width)
		{
			refreshRate =	refreshRate > modes[index].freq ?
							refreshRate : modes[index].freq;
		}

	if(refreshRate)
	{
		ds.freq = refreshRate;
		return true;
	}

	Log::Instance().error("WindowManager::validateDisplayMode", "Unable to find a fullscreen display mode matching the request");
	return false;
}

}//namespace Squirrel {
