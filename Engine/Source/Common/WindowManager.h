#pragma once

#include "Window.h"

namespace Squirrel {

#ifdef _WIN32
SQCOMMON_TEMPLATE template class SQCOMMON_API std::vector<DisplaySettings>;
SQCOMMON_TEMPLATE template class SQCOMMON_API std::vector<Window *>;
#endif
    
class SQCOMMON_API WindowManager
{
public:
	WindowManager();
	virtual ~WindowManager();

	virtual Window * createWindow(const char_t * name, const DisplaySettings& ds, bool fullScreen = true) = 0;

	virtual tuple2i getScreenResolution() const = 0;

	const std::vector<DisplaySettings>& getDisplayModes();
	bool validateDisplayMode(DisplaySettings& ds);

	void setDefaultWinMode(Window::WindowMode mode) { mDefaultWinMode = mode; }

	static WindowManager * ActiveWindowManager();
	
protected:

	virtual void enumerateDisplayModes() = 0;

    std::vector<DisplaySettings> mDisplayModes;

	std::vector<Window *> mWindows;

	//window mode which will be applied to newly created non-fullscreen window
	Window::WindowMode mDefaultWinMode;
};

}//namespace Squirrel {
