#include "WindowsClipboard.h"

namespace Squirrel {

WindowsClipboard::WindowsClipboard()
{
	mWindowHandle = NULL;
	mStringHandle = NULL;
}

WindowsClipboard::WindowsClipboard(HWND hwnd)
{
	mWindowHandle = hwnd;
	mStringHandle = NULL;
}

WindowsClipboard::~WindowsClipboard()
{
}

void WindowsClipboard::setText(const char_t * str)
{
	size_t buffSize = sizeof(char_t) * (strlen(str) + 1);
    mStringHandle = ::GlobalAlloc(GMEM_MOVEABLE, buffSize);
    if (mStringHandle)
	{
        if (::OpenClipboard (mWindowHandle))
        {
            LPVOID szString = ::GlobalLock (mStringHandle);
            if (szString)
            {
                memcpy(szString, str, buffSize);
                ::GlobalUnlock (mStringHandle);
                ::EmptyClipboard();
                ::SetClipboardData(CF_TEXT, mStringHandle);
            }
            ::CloseClipboard();
        }
	}
}

std::string WindowsClipboard::getText()
{
	std::string str;
	if (::OpenClipboard (mWindowHandle))
	{
	    mStringHandle = ::GetClipboardData(CF_TEXT);
	    if (mStringHandle)
	    {
	        str = (char_t*)::GlobalLock(mStringHandle);
	        ::GlobalUnlock(mStringHandle);
	    }
		::CloseClipboard();
	}
	return str;
}

bool WindowsClipboard::isTextAvailable()
{
	return IsClipboardFormatAvailable(CF_TEXT) == TRUE;
}


}//namespace Squirrel {
