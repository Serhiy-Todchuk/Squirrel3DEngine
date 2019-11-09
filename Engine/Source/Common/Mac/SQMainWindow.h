#import <Cocoa/Cocoa.h>

#include "../WindowDialogDelegate.h"

@interface SQMainWindow : NSWindow {
	unsigned long keyMask;
	
	Squirrel::WindowDialogDelegate * dialogDelegate;
}

- (void) setWindowDialogDelegate:(Squirrel::WindowDialogDelegate *) theDialogDelegate;

@end
