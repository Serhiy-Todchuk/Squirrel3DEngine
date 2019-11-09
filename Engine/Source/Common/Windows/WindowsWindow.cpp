#include "WindowsWindow.h"
#include "WindowsWindowManager.h"

namespace Squirrel {

WindowsWindow::WindowsWindow()
{
	mWindowHandle			= NULL;
	mDeviceContextHandle	= NULL;
	mModuleHandle			= NULL;
	mWndClassName			= "SquirrelWinClass";
	mName					= "Squirrel Engine";
	mClipboard				= NULL;
}

WindowsWindow::~WindowsWindow()
{
	if(mDeviceContextHandle != NULL)
		ReleaseDC (mWindowHandle, mDeviceContextHandle);

	if(mWindowHandle != NULL)
		DestroyWindow (mWindowHandle);

	UnregisterClass(mWndClassName.c_str(), mModuleHandle);

	DELETE_PTR(mClipboard);
}

bool WindowsWindow::createWindow(const char_t * name, const DisplaySettings& ds, WindowMode mode)
{
	mModuleHandle = GetModuleHandle(NULL);

	mName = name;

	mDisplayMode = ds;

	WNDCLASS wcl;
	wcl.hInstance       = mModuleHandle;
	wcl.lpszClassName   = mWndClassName.c_str();
	wcl.lpfnWndProc     = WindowsWindowManager::WindowEventsProcessor;
	wcl.style           = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcl.hIcon           = NULL;
	wcl.hCursor         = LoadCursor(NULL,IDC_ARROW);
	wcl.lpszMenuName    = NULL;
	wcl.cbClsExtra      = 0;
	wcl.cbWndExtra      = 0;
	wcl.hbrBackground   = (HBRUSH)COLOR_GRAYTEXT;//(HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wcl);

	DWORD ShowWndStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;//WS_VISIBLE | WS_POPUP | WS_EX_TOPMOST;
	int ShowWndMode = SW_NORMAL;//nWinMode
	if(mode == wmBorderless)
	{             
		//ShowWndMode = SW_MAXIMIZE;
		ShowWndStyle = WS_VISIBLE | WS_POPUP | WS_EX_TOPMOST;
	}

	mWindowHandle = CreateWindow(
		mWndClassName.c_str(),
		mName.c_str(),
		ShowWndStyle,
		mPosition.x,
		mPosition.y,
		mDisplayMode.width,
		mDisplayMode.height,
		HWND_DESKTOP,
		NULL,
		mModuleHandle,
		NULL);

	ShowWindow(mWindowHandle, ShowWndMode);//SW_MAXIMIZE);
	UpdateWindow(mWindowHandle);

	mDeviceContextHandle = GetDC (mWindowHandle);
	if (mDeviceContextHandle == NULL)
	{
		return false;	
	}

	RECT rcClient;
	GetClientRect(mWindowHandle, &rcClient);
	mSize = tuple2i(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

	return true;
}

void WindowsWindow::setCursorPosition(tuple2i p)
{
	POINT pt;
	pt.x = p.x;
	pt.y = p.y;
	ClientToScreen(getWindowHandle(), &pt);
	SetCursorPos(pt.x, pt.y);
}

void WindowsWindow::setWindowText(const char_t * text)
{
	::SetWindowText(getWindowHandle(), text);
}

void WindowsWindow::setPosition(tuple2i pos)
{
	SetWindowPos(mWindowHandle, 0, pos.x, pos.y, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	mPosition = pos;
}

Clipboard* WindowsWindow::getClipboard()
{
	if(mClipboard == NULL)
	{
		mClipboard = new WindowsClipboard(mWindowHandle);
	}
	return mClipboard;
}

void WindowsWindow::showDialog(const char_t * title, const char_t * text, bool yesNoDialog, WindowDialogDelegate * delegate)
{
	int ret = MessageBox(mWindowHandle, text, title, yesNoDialog ? MB_YESNO : MB_OK);
	if(delegate)
		delegate->windowDialogEnd(ret == IDYES ? WindowDialogDelegate::answerYes : WindowDialogDelegate::answerNo, (void *)title);
}

void WindowsWindow::setWindowMask(float * data, float testValue)
{
	int startx;
	INT i, j;

	HRGN Rgn, ResRgn=CreateRectRgn(0, 0, 0, 0);

	float pixel;

	//scan rows
	for(i = 0; i < mDisplayMode.height; ++i)
	{
		//i = iHeight - i -1;
		startx=-1;

		for (j = 0; j < mDisplayMode.width; ++j)
		{
			pixel=*(data +((mDisplayMode.height - i -1) * mDisplayMode.width + j));

			if (pixel < testValue)
			{
				if (startx < 0)
				{
					startx = j;
				} 
				else if (j == (mDisplayMode.width - 1))
				{
					Rgn = CreateRectRgn(startx, i, j, i+1);
					CombineRgn(ResRgn, ResRgn, Rgn, RGN_OR);
					DeleteObject(Rgn);
					startx = -1;
				}
			}
			else if (startx >= 0)
			{
				Rgn = CreateRectRgn(startx, i, j, i+1);
				CombineRgn(ResRgn, ResRgn, Rgn, RGN_OR);
				DeleteObject(Rgn);
				startx = -1;
			}
		}
	}

	SetWindowRgn(mWindowHandle, ResRgn, TRUE);

	DeleteObject(ResRgn);//?
}

}//namespace Squirrel {
