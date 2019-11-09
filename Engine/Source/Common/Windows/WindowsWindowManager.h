#pragma once

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN   1          // Exclude rarely-used stuff from Windows headers
# endif
#include <windows.h>
#include "../WindowManager.h"

namespace Squirrel {

namespace Auxiliary {
	class Application;
}

class WindowsWindow;

class SQCOMMON_API WindowsWindowManager: 
	public WindowManager
{
public:
	WindowsWindowManager();
	virtual ~WindowsWindowManager();

	virtual Window * createWindow(const char_t * name, const DisplaySettings& ds, bool fullScreen = true);

	virtual tuple2i getScreenResolution() const;

	static LRESULT CALLBACK WindowEventsProcessor(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	
	Auxiliary::Application * mApp;

private:

	virtual void enumerateDisplayModes();

	bool changeDisplayMode(const DisplaySettings& ds);

	WindowsWindow * getWindowForHandle(HWND windowHandle);

	RECT mSavedCursorClip;
	bool mRestoreCursorClip;
	bool mRestoreDisplaySettings;
};

}//namespace Squirrel {
