#import <Cocoa/Cocoa.h>
#include "SQDrawDelegate.h"

@interface SQMainView : NSView {
	SQDrawDelegate * drawDelegate;
	
	unsigned long keyMask;
}

- (id) initWithFrame:(NSRect)frameRect drawDelegate:(SQDrawDelegate*)drawHandler;

@end
