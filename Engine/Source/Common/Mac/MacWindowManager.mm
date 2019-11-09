#include "MacWindowManager.h"
#include "MacWindow.h"
#include <Common/Log.h>
#include <Common/Input.h>
#include <Render/IRender.h>
#include <map>

#import <Cocoa/Cocoa.h>

//MacWindowManager implementation

namespace Squirrel {

MacWindowManager::MacWindowManager()
{
}

MacWindowManager::~MacWindowManager()
{
}
	
uint32_t MacWindowManager::getDisplayId()
{
	CGDisplayCount dispCount = 0;
	CGDirectDisplayID dspys[32];
	CGDisplayErr theErr;
	
	memset (dspys, 0, sizeof (CGDirectDisplayID) * 32);
	
	theErr = CGGetActiveDisplayList(32, dspys, &dispCount);
	//if (theErr) return; // theErr getting list
	//if (0 == *dCnt) return; // no displays
		
	return dspys[0];
}
	
bool MacWindowManager::changeDisplayMode(const DisplaySettings& displayMode)
{	
	CGDirectDisplayID mainDisplayId = getDisplayId();
	
	CGDisplayModeRef cgDisplayModeToSet = NULL;
	
	CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(mainDisplayId, NULL);
	if(displayModes != NULL)
	{
		CFIndex count = CFArrayGetCount(displayModes);
		for(CFIndex i = 0; i < count; i++) {
			CGDisplayModeRef cgDisplayMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, i);
			int width	= (int)CGDisplayModeGetWidth(cgDisplayMode);
			int height	= (int)CGDisplayModeGetHeight(cgDisplayMode);
			if(displayMode.width == width && displayMode.height == height)
			{
				cgDisplayModeToSet = cgDisplayMode;
				break;
			}
		}
		CFRelease(displayModes);
	}
	
	if(cgDisplayModeToSet != NULL)
	{
		bool failedToApplyDisplayMode = false;
		
		CGDisplayConfigRef displayConfig = NULL;
		if(CGBeginDisplayConfiguration (&displayConfig) == kCGErrorSuccess)	{
			if(CGConfigureDisplayWithDisplayMode(displayConfig, mainDisplayId, cgDisplayModeToSet,
												 NULL) == kCGErrorSuccess) {
				failedToApplyDisplayMode = CGCompleteDisplayConfiguration(displayConfig, kCGConfigureForAppOnly) != kCGErrorSuccess;
			}
			if(failedToApplyDisplayMode)
				CGCancelDisplayConfiguration(displayConfig);
			
			return !failedToApplyDisplayMode;
		}
	}
	
	return false;
}
	
Window * MacWindowManager::createWindow(const char_t * name, const DisplaySettings& ds, bool fullScreen)
{
	DisplaySettings displayMode = ds;
 
	if(fullScreen)
	{
		tuple2i displaySize = getScreenResolution();
		bool screenHasRequestedDisplayMode = displayMode.width == displaySize.x && displayMode.height == displaySize.y;
		
		if(!screenHasRequestedDisplayMode && validateDisplayMode(displayMode))
		{
			if(!changeDisplayMode(displayMode))
			{
				Log::Instance().warning("Window->create","Can't change display settings");//Can't create a Fullscreen window
				fullScreen = false;
			}				
		}
	}

	MacWindow * window = new MacWindow();
	window->createWindow(name, displayMode, fullScreen ? Window::wmBorderless : mDefaultWinMode);

	mWindows.push_back(window);
	
	return window;
}

tuple2i MacWindowManager::getScreenResolution() const
{
	tuple2i screenSize;

	// Note, mainDisplayRect has a non-zero origin if the key window is on a secondary display
	NSRect mainDisplayRect = [[NSScreen mainScreen] frame];
	
	screenSize.x = mainDisplayRect.size.width;
	screenSize.y = mainDisplayRect.size.height;
	
	return screenSize;
}

void MacWindowManager::enumerateDisplayModes()
{
	if(mDisplayModes.size() > 0)
		return;
	
	CGDirectDisplayID mainDisplayId = getDisplayId();
	
	CFArrayRef displayModes = CGDisplayCopyAllDisplayModes(mainDisplayId, NULL);
	if(displayModes != NULL)
	{
		CFIndex count = CFArrayGetCount(displayModes);
		for(CFIndex i = 0; i < count; i++) {
			CGDisplayModeRef displayMode = (CGDisplayModeRef)CFArrayGetValueAtIndex(displayModes, (int)i);
			DisplaySettings outMode;
			outMode.width	= (int)CGDisplayModeGetWidth(displayMode);
			outMode.height	= (int)CGDisplayModeGetHeight(displayMode);
			//outMode.freq	= (int)CGDisplayModeGetRefreshRate(displayMode);
			//CFStringRef pixelMode = CGDisplayModeCopyPixelEncoding(displayMode);
			mDisplayModes.push_back(outMode);
		}
		CFRelease(displayModes);
	}
}


}//namespace Squirrel {
