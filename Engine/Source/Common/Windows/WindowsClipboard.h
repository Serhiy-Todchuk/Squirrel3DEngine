#pragma once

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN   1          // Exclude rarely-used stuff from Windows headers
# endif
#include <windows.h>

#include "../Clipboard.h"

namespace Squirrel {

class SQCOMMON_API WindowsClipboard: 
	public Clipboard
{
public:
	WindowsClipboard();
	WindowsClipboard(HWND hwnd);
	virtual ~WindowsClipboard();

	virtual void setText(const char_t * str);
	virtual std::string getText();

	virtual bool isTextAvailable();

private:
	HWND mWindowHandle;
	HGLOBAL mStringHandle;
};

}//namespace Squirrel {