#pragma once

#include "../WindowManager.h"

namespace Squirrel {

class MacWindow;

class SQCOMMON_API MacWindowManager:
	public WindowManager
{
public:
	MacWindowManager();
	virtual ~MacWindowManager();

	virtual Window * createWindow(const char_t * name, const DisplaySettings& ds, bool fullScreen = true);

	virtual tuple2i getScreenResolution() const;
	
	uint32_t getDisplayId();
private:

	bool changeDisplayMode(const DisplaySettings& ds);
	
	virtual void enumerateDisplayModes();
	
};

}//namespace Squirrel {
