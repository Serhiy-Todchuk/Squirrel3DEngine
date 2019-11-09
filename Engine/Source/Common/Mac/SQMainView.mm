
#import "SQMainView.h"

#include "../Input.h"
#include <map>

using namespace Squirrel;

Input::NamedKey ExtractKey(unichar keyCode)
{
	static std::map<unichar, Input::NamedKey> Code2KeyMap;
	if(Code2KeyMap.size() == 0)
	{
		Code2KeyMap[ 27							] = Input::Escape;
		Code2KeyMap[ ' '						] = Input::Space;
		Code2KeyMap[ NSTabCharacter				] = Input::Tab;
		Code2KeyMap[ NSUpArrowFunctionKey		] = Input::Up;
		Code2KeyMap[ NSDownArrowFunctionKey		] = Input::Down;
		Code2KeyMap[ NSLeftArrowFunctionKey		] = Input::Left;
		Code2KeyMap[ NSRightArrowFunctionKey	] = Input::Right;
		Code2KeyMap[ NSEnterCharacter			] = Input::Enter;
		Code2KeyMap[ NSCarriageReturnCharacter	] = Input::Enter;
		Code2KeyMap[ NSBackspaceCharacter		] = Input::Backspace;
		Code2KeyMap[ NSDeleteCharacter			] = Input::Delete;
		Code2KeyMap[ NSPauseFunctionKey			] = Input::Pause;
	}
	
	std::map<unichar, Input::NamedKey>::iterator itKey = Code2KeyMap.find( keyCode );
	if(itKey != Code2KeyMap.end())
	{
		return itKey->second;
	}
	
	if(keyCode >= '0' && keyCode <= '9')
	{
		return (Input::NamedKey)(Input::Key0 + (keyCode - '0'));
	}
	
	if(keyCode >= 'a' && keyCode <= 'z')
	{
		return (Input::NamedKey)(Input::KeyA + (keyCode - 'a'));
	}
	
	if(keyCode >= 'A' && keyCode <= 'Z')
	{
		return (Input::NamedKey)(Input::KeyA + (keyCode - 'A'));
	}
	
	if(keyCode >= NSF1FunctionKey && keyCode <= NSF15FunctionKey)
	{
		return (Input::NamedKey)(Input::F1 + (keyCode - NSF1FunctionKey));
	}
	
	return Input::Undefined;
}

@implementation SQMainView

- (id) initWithFrame:(NSRect)frameRect drawDelegate:(SQDrawDelegate*)drawHandler
{
	if (self = [super initWithFrame:frameRect]) {
		
		keyMask = 0;
		
        drawDelegate = drawHandler;
		
		// Look for changes in view size
		// Note, -reshape will not be called automatically on size changes because NSView does not export it to override 
		[[NSNotificationCenter defaultCenter] addObserver:self 
												 selector:@selector(reshape) 
													 name:NSViewGlobalFrameDidChangeNotification
												   object:self];
	}
	
	return self;
}

- (void) reshape
{
    if(drawDelegate != NULL)
    {
        drawDelegate->resize([self bounds].size.width, [self bounds].size.height);
    }
}

- (void) drawRect:(NSRect)dirtyRect
{
    if(drawDelegate != NULL)
    {
        drawDelegate->draw();
    }
}


- (BOOL) acceptsFirstResponder
{
    // We want this view to be able to receive key events
    return YES;
}

- (BOOL)keyWasDown:(unsigned long)mask
{
    return (keyMask & mask) == mask;
}

- (BOOL)isMask:(unsigned long)newMask upEventForModifierMask:(unsigned long)mask
{
    return [self keyWasDown:mask] && ((newMask & mask) == 0x0000);
}

- (BOOL)isMask:(unsigned long)newMask downEventForModifierMask:(unsigned long)mask
{
    return ![self keyWasDown:mask] && ((newMask & mask) == mask);
}

- (void)processFlagKey:(unsigned long)key sqKey:(Input::NamedKey)sqKeyCode event:(NSEvent *)theEvent
{
	if([self isMask:[theEvent modifierFlags] downEventForModifierMask:key])
    {
		Input::Get()->pushDownKey(sqKeyCode);
		keyMask |= key;
    }
	else if([self isMask:[theEvent modifierFlags] upEventForModifierMask:key])
    {
		Input::Get()->pushUpKey(sqKeyCode);
		keyMask ^= key;
    }
}

- (void)flagsChanged:(NSEvent *)theEvent
{
	[self processFlagKey:NSAlternateKeyMask sqKey:Input::LAlt event:theEvent];
	[self processFlagKey:NSShiftKeyMask sqKey:Input::LShift event:theEvent];
	[self processFlagKey:NSControlKeyMask sqKey:Input::LControl event:theEvent];
}

- (void) keyDown:(NSEvent *)theEvent
{
    NSString * characters = [theEvent charactersIgnoringModifiers];
	unichar character = [characters characterAtIndex: 0];
	
	Input::NamedKey sqKey = ExtractKey(character);
	Input::Get()->pushDownKey(sqKey);
	
	characters = [theEvent characters];
	for(int i = 0; i < [characters length]; ++i)
	{
		character = [characters characterAtIndex: 0];
		Input::Get()->pushCharacter((char_t)character);
	}
}

- (void) keyUp:(NSEvent *)theEvent
{
    NSString * characters = [theEvent charactersIgnoringModifiers];
	unichar character = [characters characterAtIndex: 0];
	
	Input::NamedKey sqKey = ExtractKey(character);
	Input::Get()->pushUpKey(sqKey);
}

- (void)mouseDown:(NSEvent *)theEvent
{
	Input::Get()->pushDownKey(Input::Mouse0);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	Input::Get()->pushUpKey(Input::Mouse0);
	
	if([theEvent clickCount] > 1)
	{
		Input::Get()->setDoubleClicked();
	}
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	Input::Get()->pushDownKey(Input::Mouse1);
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	Input::Get()->pushUpKey(Input::Mouse1);
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
	NSInteger buttonNumber = [theEvent buttonNumber];
	Input::Get()->pushDownKey((Input::NamedKey)(Input::Mouse0 + buttonNumber));
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
	NSInteger buttonNumber = [theEvent buttonNumber];
	Input::Get()->pushUpKey((Input::NamedKey)(Input::Mouse0 + buttonNumber));
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	NSPoint eventLocation = [theEvent locationInWindow];
	NSPoint localPoint = [self convertPoint:eventLocation fromView:nil];
	Input::Get()->setMousePos(tuple2i(localPoint.x, [self bounds].size.height - localPoint.y));
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[self mouseMoved:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
	[self mouseMoved:theEvent];
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
	[self mouseMoved:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	float scroll = [theEvent deltaY];
	Input::Get()->setMouseScroll(scroll);
}
 
- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self 
													name:NSViewGlobalFrameDidChangeNotification
												  object:self];
	[super dealloc];
}	

@end
