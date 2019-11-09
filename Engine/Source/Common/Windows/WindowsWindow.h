#pragma once

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN   1          // Exclude rarely-used stuff from Windows headers
# endif
#include <windows.h>

#include "../Window.h"
#include "WindowsClipboard.h"

namespace Squirrel {

class SQCOMMON_API WindowsWindow:
	public Window
{
public:
	WindowsWindow();
	virtual ~WindowsWindow();

	virtual bool createWindow(const char_t * name, const DisplaySettings& ds, WindowMode mode);
	virtual void setWindowMask(float * data, float testValue);
	virtual void setCursorPosition(tuple2i pt);
	virtual void setWindowText(const char_t * text);
	virtual void setPosition(tuple2i pos);

	virtual Clipboard* getClipboard();

	virtual void showDialog(const char_t * title, const char_t * text, bool yesNoDialog = false, WindowDialogDelegate * delegate = NULL);

	HWND		getWindowHandle()			{ return mWindowHandle; }
	HDC			getDeviceContextHandle()	{ return mDeviceContextHandle; }

	void sizeChanged(tuple2i sz) { mSize = sz; }

private:

	HWND		mWindowHandle;
	HDC			mDeviceContextHandle;
	HINSTANCE	mModuleHandle;

	std::string mWndClassName;
	std::string mName;

	WindowsClipboard * mClipboard;
};

}//namespace Squirrel {