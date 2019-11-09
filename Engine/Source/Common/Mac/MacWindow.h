#pragma once

#include "../Window.h"
#include "MacClipboard.h"
#include "SQDrawDelegate.h"

namespace Squirrel {

class SQCOMMON_API MacWindow:
	public Window, public SQDrawDelegate
{
public:
	MacWindow();
	virtual ~MacWindow();

	virtual bool createWindow(const char_t * name, const DisplaySettings& ds, WindowMode mode);
	virtual void setWindowMask(float * data, float testValue);
	virtual void setCursorPosition(tuple2i pt);
	virtual void setWindowText(const char_t * text);
	virtual void setPosition(tuple2i pos);

	virtual Clipboard* getClipboard();

	virtual void showDialog(const char_t * title, const char_t * text, bool yesNoDialog = false, WindowDialogDelegate * delegate = NULL);

	void sizeChanged(tuple2i sz) { mSize = sz; }
	
	void * getWindowView() { return mNSView; }

private:
	
	void draw();
    void resize(int x, int y);

	std::string mName;

	MacClipboard * mClipboard;
	
	void * mNSWindow;
	void * mNSWindowController;
	void * mNSView;
	void * mNSImage;
	
	
};

}//namespace Squirrel {