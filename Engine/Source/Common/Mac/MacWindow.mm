#include "MacWindow.h"
#include "MacWindowManager.h"
#include "MacClipboard.h"

#import <Cocoa/Cocoa.h>
#import "SQMainView.h"
#import "SQMainWindow.h"

namespace Squirrel {

MacWindow::MacWindow()
{
	mClipboard			= NULL;
	mNSWindow			= NULL;
	mNSView				= NULL;
	mNSWindowController	= NULL;
	mNSImage			= NULL;
}

MacWindow::~MacWindow()
{
	DELETE_PTR(mClipboard);
	
	if(mNSWindow)
		[(NSWindow *)mNSWindow release];
}

bool MacWindow::createWindow(const char_t * name, const DisplaySettings& ds, WindowMode mode)
{
	mName = name;

	mDisplayMode = ds;

	NSRect windowRect, viewRect;
	
	int windowStyleMask = NSTitledWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;// NSClosableWindowMask
	
	if(mode == wmBorderless)
	{
		windowStyleMask = NSBorderlessWindowMask;
	}
	
	windowRect = NSMakeRect(0.0, 0.0, ds.width, ds.height);
	
	// Create a screen-sized window on the display you want to take over
	NSWindow * window = [[SQMainWindow alloc] initWithContentRect:windowRect styleMask:windowStyleMask
													 backing:NSBackingStoreBuffered defer:YES];
	
	// Set the window level to be above the menu bar
	[window setLevel:NSMainMenuWindowLevel+1];
	
	// Perform any other window configuration you desire
	[window setOpaque:YES];
	[window setHidesOnDeactivate:YES];
	
	// Create a view with a double-buffered OpenGL context and attach it to the window
	// By specifying the non-fullscreen context as the shareContext, we automatically inherit the OpenGL objects (textures, etc) it has defined
	viewRect = NSMakeRect(0.0, 0.0, windowRect.size.width, windowRect.size.height);
	NSView * view = [[SQMainView alloc] initWithFrame:viewRect drawDelegate:this];
	[window setContentView:view];
	[window makeFirstResponder:view];
	[window setAcceptsMouseMovedEvents:YES];
	
	// Show the window
	[window makeKeyAndOrderFront:nil];
	
	// Set the scene with the full-screen viewport and viewing transformation
	mSize = tuple2i(windowRect.size.width, windowRect.size.height);
	
	// Assign the view's MainController to self
	//[window setMainController:self];
	
	// Mark the view as needing drawing to initalize its contents
	[view setNeedsDisplay:YES];
	
	mNSWindow = window;
	mNSView = view;
	
	//setPosition(tuple2i(0,0));
	
	return true;
}

void MacWindow::draw()
{
	if(mNSImage)
	{
		//TODO: look at CGContextClipToMask!!!
		
		// Clear the drawing rect.
		//[[NSColor clearColor] set];
		//NSRectFill([(NSView *)mNSView frame]);
        //[(NSImage *)mNSImage drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeSourceOut fraction:0.5f];

		// Reset the window shape and shadow.
		//if (shouldDisplayWindow)
		{
		//	[(NSWindow *)mNSWindow display];
		//	[(NSWindow *)mNSWindow setHasShadow:NO];
		//	[(NSWindow *)mNSWindow setHasShadow:YES];
		}
	}
}
	
void MacWindow::resize(int x, int y)
{
	mSize = tuple2i(x, y);
}
	
void MacWindow::setCursorPosition(tuple2i p)
{
	NSRect rect = [(NSWindow *)mNSWindow convertRectToScreen: NSMakeRect(p.x, p.y, 0, 0)];
	
	CGWarpMouseCursorPosition(rect.origin);
}

void MacWindow::setWindowText(const char_t * text)
{
	[(NSWindow *)mNSWindow setTitle:[NSString stringWithCString:text encoding:NSASCIIStringEncoding]];
}

void MacWindow::setPosition(tuple2i pos)
{
	mPosition = pos;
	
	[(NSWindow *)mNSWindow setFrameTopLeftPoint: NSMakePoint(pos.x, pos.y)];
}

Clipboard* MacWindow::getClipboard()
{
	if(mClipboard == NULL)
	{
		mClipboard = new MacClipboard();
	}
	return mClipboard;
}

void MacWindow::showDialog(const char_t * title, const char_t * text, bool yesNoDialog, WindowDialogDelegate * delegate)
{
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:yesNoDialog ? @"Yes" : @"OK"];
	if(yesNoDialog)
		[alert addButtonWithTitle:@"No"];
	[alert setMessageText:[NSString stringWithCString:title encoding:NSASCIIStringEncoding]];
	[alert setInformativeText:[NSString stringWithCString:text encoding:NSASCIIStringEncoding]];
	[alert setAlertStyle:NSWarningAlertStyle];
	[alert beginSheetModalForWindow:(NSWindow *)mNSWindow
					  modalDelegate:(NSWindow *)mNSWindow
					 didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
						contextInfo:(void *)title];
	
	[(SQMainWindow *)mNSWindow setWindowDialogDelegate: delegate];
}

void MacWindow::setWindowMask(float * data, float testValue)
{
	NSSize imageSize = [(NSView *)mNSView bounds].size;
	
	NSBitmapImageRep * imageRep = NULL;
	
	//ensure image is initialized
	
	if(mNSImage == nullptr)
	{
		NSImage * image = [[NSImage alloc] initWithSize:imageSize];
		
		imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nullptr pixelsWide:imageSize.width pixelsHigh:imageSize.height bitsPerSample:8 samplesPerPixel:2 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceWhiteColorSpace bitmapFormat:NSAlphaNonpremultipliedBitmapFormat bytesPerRow:imageSize.width * 2 bitsPerPixel:0];
		
		[image addRepresentation:imageRep];
		
		mNSImage = image;
	}
	
	if(!imageRep)
	{
		imageRep = [[(NSImage *)mNSImage representations] objectAtIndex:0];
	}
	
	//build mask
	
	int w = (int)imageSize.width;
	int h = (int)imageSize.height;
	
	float pixel;
	NSUInteger mask[2];
	
	for(int x = 0; x < w; ++x)
	{
		for(int y = 0; y < h; ++y)
		{
			pixel = *(data + ((h - y -1) * w + x));
			
			mask[0] = mask[1] = (pixel < testValue) ? 255 : 0;
			
			[imageRep setPixel:mask atX:x y:y];
		}
	}
}

}//namespace Squirrel {
