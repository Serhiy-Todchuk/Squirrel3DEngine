
#import "SQMainWindow.h"

@implementation SQMainWindow

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation
{
	keyMask = 0;
	dialogDelegate = 0;
	self = [super initWithContentRect:contentRect styleMask:windowStyle backing:bufferingType defer:deferCreation];
	return self;
}

/*
 I found out what was preventing the keyDown event from being called. It was the NSBorderlessWindowMask mask, it prevents the window from become the key and main window.
 */

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

- (void) alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	if(dialogDelegate)
	{
		dialogDelegate->windowDialogEnd(returnCode == NSAlertFirstButtonReturn ? 0 : 1, contextInfo);
		dialogDelegate = nullptr;
	}
}

- (void) setWindowDialogDelegate:(Squirrel::WindowDialogDelegate *) theDialogDelegate
{
	dialogDelegate = theDialogDelegate;
}

@end
