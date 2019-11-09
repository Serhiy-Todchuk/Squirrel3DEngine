#pragma once

#include <vector>
#include "Context.h"
#include "Clipboard.h"
#include "tuple.h"
#include "macros.h"
#include "WindowDialogDelegate.h"

#ifdef	_WIN32
//	disable warning on extern before template instantiation
#	pragma warning( disable: 4231 )
#endif

namespace Squirrel {

//TODO rename namespace and project to Framework

class SQCOMMON_API DisplaySettings
{
public:
	DisplaySettings(): freq(60), width(800), height(600), colorBits(32)	{	}
	DisplaySettings(int w, int h, int c, int f): freq(f), width(w), height(h), colorBits(c)	{	}
	~DisplaySettings() {}

	void set(int width, int height, int colorBits, int freq)	{
		this->width		= width;
		this->height	= height;
		this->colorBits	= colorBits;
		this->freq		= freq;
	}

public:
	int freq;
	int width;
	int height;
	int colorBits;
};
	
class SQCOMMON_API Window
{
public:
	enum WindowMode
	{
		wmBorderless,
		wmStandard
	};

public:
	Window();
	virtual ~Window();

	const DisplaySettings& getDisplaySettings()	const	{ return mDisplayMode; }
	tuple2i getPosition()						const	{ return mPosition; }
	tuple2i getSize()							const	{ return mSize; }
	bool isFullscreen()							const	{ return mIsFullscreen; }

	virtual bool createWindow(const char_t * name, const DisplaySettings& ds, WindowMode mode)	= 0;
	virtual void setWindowMask(float * data, float testValue)				= 0;
	virtual void setCursorPosition(tuple2i pt)								= 0;
	virtual void setWindowText(const char_t * text)							= 0;
	virtual void setPosition(tuple2i pos)									= 0;

	virtual Clipboard* getClipboard() = 0;

	virtual void showDialog(const char_t * title, const char_t * text, bool yesNoDialog = false, WindowDialogDelegate * delegate = NULL) = 0;

	void registerContext(Context * ctx);

	void setCurrentContext(Context * ctx);
	Context * getCurrentContext() { return mCurrentContext; }

	std::vector<Context *>& getContexts() { return mRenderingContexts; }

	bool	isMainWindow() { return mMainWindow; }
	void	setMainWindow() { mMainWindow = true; }

protected:

	tuple2i			mPosition;
	tuple2i			mSize;
	DisplaySettings	mDisplayMode;
	bool			mIsFullscreen;
	WindowMode		mWindowMode; 

	std::vector<Context *>	mRenderingContexts;

	Context * mCurrentContext;

	bool		mMainWindow;
};

}//namespace Squirrel {