#include "MacImageLoader.h"

#import <Cocoa/Cocoa.h>

namespace Squirrel {

namespace RenderData { 

Image * MacImageLoader::LoadImage(Data * fromData)
{
	NSData * nsData = [[NSData alloc] initWithBytesNoCopy:fromData->getData() length:fromData->getLength() freeWhenDone:NO];
	
	NSImage * nsImage = [[NSImage alloc] initWithData:nsData];
	
	if(!nsImage)
	{
		[nsData release];
		return nullptr;
	}
	
	NSBitmapImageRep * nsImageRep = [[nsImage representations] objectAtIndex:0];
	
	NSInteger width   = [nsImageRep pixelsWide];
	NSInteger height  = [nsImageRep pixelsHigh];
	
	NSInteger samplesPerPixel = [nsImageRep samplesPerPixel];
	NSInteger bitsPerPixel = [nsImageRep bitsPerPixel];
	NSInteger bitsPerSample = bitsPerPixel / samplesPerPixel;
	bool hasAlpha = [nsImageRep hasAlpha];
	
	Image::Format format = Image::RGBA;
	
	switch (samplesPerPixel) {
		case 1:
			format = hasAlpha ? Image::Alpha : Image::Luminance;
			break;
		case 2:
			format = Image::LuminanceAlpha;
			break;
		case 3:
			format = Image::RGB;
			break;
		case 4:
			format = Image::RGBA;
			break;
		default:
			break;
	}
	
	Image::DataType dataType = Image::Int8;

	switch (bitsPerSample) {
		case 8:
			dataType = Image::Int8;
			break;
		case 16:
			dataType = Image::Int16;
			break;
			
		default:
			break;
	}
	
	Image * image = new Image(width, height, 1, dataType, format);
	
	for(int x = 0; x < width; ++x)
	{
		for(int y = 0; y < height; ++y)
		{
			NSUInteger inPixel[4];
			[nsImageRep getPixel:inPixel atX:x y:y];
			byte * outPixel = image->getPixel(x, height - y - 1);
			
			if(dataType == Image::Int8)
			{
				outPixel[0] = static_cast<byte>(inPixel[0]);
				if(samplesPerPixel > 1)
					outPixel[1] = static_cast<byte>(inPixel[1]);
				if(samplesPerPixel > 2)
					outPixel[2] = static_cast<byte>(inPixel[2]);
				if(samplesPerPixel > 3)
					outPixel[3] = static_cast<byte>(inPixel[3]);
			}
			else if(dataType == Image::Int16)
			{
				uint16* outPixel16 = reinterpret_cast<uint16*>(outPixel);
				outPixel16[0] = static_cast<uint16>(inPixel[0]);
				if(samplesPerPixel > 1)
					outPixel16[1] = static_cast<uint16>(inPixel[1]);
				if(samplesPerPixel > 2)
					outPixel16[2] = static_cast<uint16>(inPixel[2]);
				if(samplesPerPixel > 3)
					outPixel16[3] = static_cast<uint16>(inPixel[3]);
			}
		}
	}
	
	[nsImage release];
	[nsData release];
	
	return image;
}

}//namespace RenderData { 

}//namespace Squirrel {

