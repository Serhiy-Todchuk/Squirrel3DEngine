#include "WindowsWindowManager.h"
#include "WindowsWindow.h"
#include <Common/Log.h>
#include <Common/Input.h>
#include <Render/IRender.h>
#include <map>
#include <windowsx.h>
#include <Auxiliary/Application.h>

namespace Squirrel {

#define VK_KEY_0	0x30
#define VK_KEY_9	0x39

#define VK_KEY_A	0x41
#define VK_KEY_Z	0x5A

Input::NamedKey ExtractKey(DWORD keyCode)
{
	static std::map<DWORD, Input::NamedKey> Code2KeyMap;
	if(Code2KeyMap.size() == 0)
	{
		Code2KeyMap[ VK_LBUTTON	] = Input::Mouse0;
		Code2KeyMap[ VK_RBUTTON	] = Input::Mouse1;
		Code2KeyMap[ VK_MBUTTON	] = Input::Mouse2;
		Code2KeyMap[ VK_ESCAPE	] = Input::Escape;
		Code2KeyMap[ VK_SPACE	] = Input::Space;
		Code2KeyMap[ VK_TAB		] = Input::Tab;
		Code2KeyMap[ VK_UP		] = Input::Up;
		Code2KeyMap[ VK_DOWN	] = Input::Down;
		Code2KeyMap[ VK_LEFT	] = Input::Left;
		Code2KeyMap[ VK_RIGHT	] = Input::Right;
		Code2KeyMap[ VK_RETURN	] = Input::Enter;
		Code2KeyMap[ VK_BACK	] = Input::Backspace;
		Code2KeyMap[ VK_DELETE	] = Input::Delete;
		Code2KeyMap[ VK_SHIFT	] = Input::LShift;
		Code2KeyMap[ VK_CONTROL	] = Input::LControl;
		Code2KeyMap[ VK_PAUSE	] = Input::Pause;
	}

	std::map<DWORD, Input::NamedKey>::iterator itKey = Code2KeyMap.find( keyCode );
	if(itKey != Code2KeyMap.end())
	{
		return itKey->second;
	}

	if(keyCode >= VK_KEY_0 && keyCode <= VK_KEY_9)
	{
		return (Input::NamedKey)(Input::Key0 + (keyCode - VK_KEY_0));
	}

	if(keyCode >= VK_KEY_A && keyCode <= VK_KEY_Z)
	{
		return (Input::NamedKey)(Input::KeyA + (keyCode - VK_KEY_A));
	}

	if(keyCode >= VK_F1 && keyCode <= VK_F15)
	{
		return (Input::NamedKey)(Input::F1 + (keyCode - VK_F1));
	}

	return Input::Undefined;
}

LRESULT CALLBACK WindowsWindowManager::WindowEventsProcessor(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	Input::NamedKey key = Input::Undefined;
	std::string::value_type character = 0;
	tuple2i mousePos;
	float mouseScroll = 0;
	POINT cur;
	RECT rcClient;

	GetCursorPos(&cur);
	mousePos.x = cur.x;
	mousePos.y = cur.y;
	if(mousePos != Input::Get()->getMousePos())
	{
		//Input::Get()->setMousePos( mousePos );
	}

	WindowsWindow * window = static_cast<WindowsWindowManager *>(ActiveWindowManager())->getWindowForHandle(hWnd);

	switch(wMsg)
	{
	case WM_CREATE:
			break;
	case WM_DESTROY:
		if(window != NULL && window->isMainWindow())
			PostQuitMessage(0);
			break;
	case WM_ACTIVATE:
		if(LOWORD(wParam) != WA_INACTIVE)
		{
			if(static_cast<WindowsWindowManager *>(ActiveWindowManager())->mApp)
			{
				static_cast<WindowsWindowManager *>(ActiveWindowManager())->mApp->onBecomeActive();
			}
		}
		break;
	case WM_LBUTTONDOWN:	Input::Get()->pushDownKey(	Input::Mouse0 ); break;
	case WM_LBUTTONUP:		Input::Get()->pushUpKey(	Input::Mouse0 ); break;
	case WM_RBUTTONDOWN:	Input::Get()->pushDownKey(	Input::Mouse1 ); break;
	case WM_RBUTTONUP:		Input::Get()->pushUpKey(	Input::Mouse1 ); break;
	case WM_MBUTTONDOWN:	Input::Get()->pushDownKey(	Input::Mouse2 ); break;
	case WM_MBUTTONUP:		Input::Get()->pushUpKey(	Input::Mouse2 ); break;
	case WM_LBUTTONDBLCLK:	Input::Get()->setDoubleClicked(); break;
	case WM_KEYDOWN:
		key = ExtractKey( wParam );
		if(key != Input::Undefined)
		{
			Input::Get()->pushDownKey( key );
		}
		break;
	case WM_KEYUP:
		key = ExtractKey( wParam );
		if(key != Input::Undefined)
		{
			Input::Get()->pushUpKey( key );
		}
		break;
	case WM_CHAR:
		character = (std::string::value_type)wParam;
		Input::Get()->pushCharacter( character );
		break;
	case WM_SIZE:
		if(window != NULL)
		{
			GetClientRect(hWnd, &rcClient);
			tuple2i newSize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
			window->sizeChanged(newSize);
		}
		break;
	case WM_MOUSEMOVE:
		cur.x = GET_X_LPARAM(lParam);
		cur.y = GET_Y_LPARAM(lParam);
		//ClientToScreen(hWnd, &cur);
		mousePos.x = cur.x;
		mousePos.y = cur.y;
		Input::Get()->setMousePos( mousePos );
		break;
	case WM_MOUSEWHEEL:
		mouseScroll = GET_WHEEL_DELTA_WPARAM(wParam);
		mouseScroll /= WHEEL_DELTA;
		Input::Get()->setMouseScroll( mouseScroll );
		break;
	default:
		return DefWindowProc( hWnd,wMsg,wParam,lParam );
	}
	return 0;
}

WindowsWindowManager::WindowsWindowManager()
{
	mApp = NULL;
	mRestoreCursorClip		= false;
	mRestoreDisplaySettings	= false;
}

WindowsWindowManager::~WindowsWindowManager()
{
	if(mRestoreCursorClip)
	{
		ClipCursor(&mSavedCursorClip);
	}
	if(mRestoreDisplaySettings)
	{
		ChangeDisplaySettings(NULL,0);           // If So Switch Back To The Desktop
	}
	ShowCursor(true);                        // Show Mouse Pointer
}

WindowsWindow * WindowsWindowManager::getWindowForHandle(HWND windowHandle)
{
	for(int i = 0; i < mWindows.size(); ++i)
	{
		WindowsWindow * wnd = static_cast<WindowsWindow *>(mWindows[i]);
		if(wnd->getWindowHandle() == windowHandle)
			return wnd;
	}

	return NULL;
}

bool WindowsWindowManager::changeDisplayMode(const DisplaySettings& ds)
{
	DEVMODE dmScreenSettings; 
	ZeroMemory(&dmScreenSettings,sizeof(dmScreenSettings));
	dmScreenSettings.dmDisplayFrequency = ds.freq;
	dmScreenSettings.dmBitsPerPel       = ds.colorBits;
	dmScreenSettings.dmPelsHeight       = ds.height;
	dmScreenSettings.dmPelsWidth        = ds.width;
	dmScreenSettings.dmFields           = DM_BITSPERPEL | DM_PELSWIDTH |
										  DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	dmScreenSettings.dmSize             = sizeof(dmScreenSettings);

	if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return false;
	}

	return true;
}

Window * WindowsWindowManager::createWindow(const char_t * name, const DisplaySettings& ds, bool fullScreen)
{
	DisplaySettings displayMode = ds;
	
	if(fullScreen)
	{
		tuple2i displaySize = getScreenResolution();
		bool screenHasRequestedDisplayMode = displayMode.width == displaySize.x && displayMode.height == displaySize.y;
	 
		if(!screenHasRequestedDisplayMode && validateDisplayMode(displayMode))
		{
			GetClipCursor(&mSavedCursorClip);

			if(changeDisplayMode(displayMode))
			{
				RECT newCursorClip;
				SetRect(&newCursorClip, 0, 0, displayMode.width, displayMode.height);
				ClipCursor(&newCursorClip);

				mRestoreCursorClip = true;
				mRestoreDisplaySettings = true;
			}
			else
			{
				Log::Instance().warning("Window->create","Can't change display settings");//Can't create a Fullscreen window
				fullScreen = false;
			}
		}
	}

	WindowsWindow * window = new WindowsWindow();
	window->createWindow(name, displayMode, fullScreen ? Window::wmBorderless : mDefaultWinMode);

	mWindows.push_back(window);

	return window;
}

tuple2i WindowsWindowManager::getScreenResolution() const
{
	tuple2i screenSize;
	screenSize.x = GetSystemMetrics(SM_CXSCREEN);
	screenSize.y = GetSystemMetrics(SM_CYSCREEN);
	return screenSize;
}

void WindowsWindowManager::enumerateDisplayModes()
{
	if(mDisplayModes.size() > 0)
		return;

	int j = 0;
	DEVMODE deviceMode;
	ZeroMemory(&deviceMode, sizeof(DEVMODE));
	deviceMode.dmSize = sizeof(DEVMODE);
	while(EnumDisplaySettings(NULL, j++, &deviceMode))
	{
		if (deviceMode.dmBitsPerPel > 8 && ChangeDisplaySettings(&deviceMode, CDS_FULLSCREEN | CDS_TEST) == DISP_CHANGE_SUCCESSFUL) 
		{
			DisplaySettings mode;
			mode.set(deviceMode.dmPelsWidth, deviceMode.dmPelsHeight, deviceMode.dmBitsPerPel, deviceMode.dmDisplayFrequency);
			mDisplayModes.push_back(mode);
		}
	}
}


}//namespace Squirrel {
