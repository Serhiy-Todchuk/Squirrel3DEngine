#include "MacUtils.h"
#import <Cocoa/Cocoa.h>

namespace Squirrel {
	
void * MacUtils::OpenAutoreleasePool()
{
	return (void *)[[NSAutoreleasePool alloc] init];
}
	
void MacUtils::CloseAutoreleasePool(void * pool)
{
	[(NSAutoreleasePool *)pool release];
}
	
std::string MacUtils::GetBundleName()
{
	NSBundle* bundle = [NSBundle mainBundle];
	NSString* path = [[bundle bundlePath] lastPathComponent];

	const int strBuffSize = 256;
	char_t strBuff[strBuffSize];

	[path getCString:strBuff maxLength:strBuffSize encoding:NSASCIIStringEncoding];

	return strBuff;
}
	
std::string MacUtils::GetBundleParentFolder()
{
	NSBundle* bundle = [NSBundle mainBundle];
	NSString* path = [[bundle bundlePath] stringByDeletingLastPathComponent];

	const int strBuffSize = 256;
	char_t strBuff[strBuffSize];

	[path getCString:strBuff maxLength:strBuffSize encoding:NSASCIIStringEncoding];

	return strBuff;
}
	
void MacUtils::TerminateProcess()
{
	[NSApp terminate:nil];
}
	
void MacUtils::DebugLog(const char_t * str)
{
	NSLog(@"%s", str);
}
	
}//namespace Squirrel {

