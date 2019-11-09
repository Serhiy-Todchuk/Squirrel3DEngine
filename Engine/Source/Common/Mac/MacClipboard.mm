#include "MacClipboard.h"
#import <Cocoa/Cocoa.h>

namespace Squirrel {

MacClipboard::MacClipboard()
{
}

MacClipboard::~MacClipboard()
{
}

void MacClipboard::setText(const char_t * str)
{
	NSString * pboardStr = [[NSString alloc] initWithCString:str encoding:NSASCIIStringEncoding];
	
	if(pboardStr == nil)
		return;
	
	[[NSPasteboard generalPasteboard] setString:pboardStr forType:NSStringPboardType];
	
	[pboardStr release];
}

std::string MacClipboard::getText()
{
	NSString * pboardStr = [[NSPasteboard generalPasteboard] stringForType:NSStringPboardType];
	if(pboardStr == nil)
		return std::string();
	
	const char_t * text = [pboardStr cStringUsingEncoding:NSASCIIStringEncoding];
	if(text == NULL)
		return std::string();
	
	return std::string(text);
}

bool MacClipboard::isTextAvailable()
{
	return [[NSPasteboard generalPasteboard] stringForType:NSStringPboardType] != nil;
}

}//namespace Squirrel {
