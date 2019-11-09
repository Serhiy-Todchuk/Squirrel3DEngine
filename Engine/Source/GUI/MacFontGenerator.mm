//-----------------------------------------------------------------------------
// Copyright (c) 2007-2008 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "MacFontGenerator.h"
#include <Render/Image.h>

#import <Cocoa/Cocoa.h>

namespace Squirrel {
namespace GUI {

using namespace RenderData;

MacFontGenerator::MacFontGenerator()
{
}

MacFontGenerator::~MacFontGenerator()
{
}

Font * MacFontGenerator::create(const char *name, int size, Font::Style style)
{
	float fontSize = float(size) * 1.5f;
	
	NSAutoreleasePool * autoreleasePool = [[NSAutoreleasePool alloc] init];
	
	font = new Font();

    font->mName = name;
    font->mPointSize = size;

	NSString * nsFontName = [NSString stringWithCString:name encoding:NSASCIIStringEncoding];
	NSFont * nsFont = [NSFont fontWithName: nsFontName size: fontSize];
	
	if(!nsFont)
	{
		nsFont = [NSFont fontWithName: @"Helvetica" size: fontSize];
	}

	//obtain font metrics
	
	const int firstChar = 32, lastChar = 126;
	const int charsNum = lastChar - firstChar + 1;
	
	int c, x, y;
	
	char allCharacters[charsNum + 1];
	
	allCharacters[charsNum] = '\0';
	
	for(c = firstChar; c <= lastChar; ++c)
	{
		allCharacters[c - firstChar] = c;
	}
	
	NSString * allCharsStr = [NSString stringWithCString:allCharacters encoding:NSASCIIStringEncoding];
	
	/* put the string's text into a text storage
	 so we can access the glyphs through a layout. */
	NSTextStorage *textStore = [[[NSTextStorage alloc] initWithString:allCharsStr] autorelease];
	NSTextContainer *textContainer = [[[NSTextContainer alloc] init] autorelease];
	NSLayoutManager *myLayout = [[[NSLayoutManager alloc] init] autorelease];
	[myLayout addTextContainer:textContainer];
	[textStore addLayoutManager:myLayout];
	[textStore setFont: nsFont];
	
	font->mCharHeight = [myLayout defaultLineHeightForFont:nsFont];
 	font->mCellHeight = font->mCharHeight;
	font->mCharMaxWidth = 0;
    font->mCharAvgWidth = 0;
	
	for(c = firstChar; c <= lastChar; ++c)
	{
		NSGlyph nsGlyph = [myLayout glyphAtIndex:c - firstChar];
		
		NSSize advancement = [nsFont advancementForGlyph:nsGlyph];
		
		float width = advancement.width;
		
		if (width > font->mCharMaxWidth)
            font->mCharMaxWidth = width;
		
        font->mCharAvgWidth += width;
        font->mGlyphs[c - firstChar].width = width;
	}
	
	font->mCharAvgWidth /= charsNum;
    font->mCellWidth = font->mCharMaxWidth + 1;

	//render glyphs
	
	NSMutableDictionary *strFontAttribs = [NSMutableDictionary dictionary];
    [strFontAttribs setObject: nsFont forKey: NSFontAttributeName];
    [strFontAttribs setObject: [NSColor whiteColor] forKey: NSForegroundColorAttributeName];
	
	const int gridX = 10, gridY = 10;
	
    int w = Math::getNextPowerOfTwo(gridX * font->mCellWidth);
    int h = Math::getNextPowerOfTwo(gridY * font->mCellHeight);
	
	NSSize imageSize = NSMakeSize(w, h);
	
	NSImage * image = [[NSImage alloc] initWithSize:imageSize];
	
	[image lockFocus];
	[[NSGraphicsContext currentContext] setShouldAntialias: YES];
		
	[[NSColor blackColor] set];
	
	float vOffset = 1.0f - (font->mCellHeight * gridY) / h;
	
	x = y = 0;
	for(c = firstChar; c <= lastChar; ++c)
	{
		// render glyph into image
		
		char_t str[2] = {(char_t)c, 0};
		NSString *nsString = [[NSString alloc] initWithCString:str encoding:NSASCIIStringEncoding];
		NSPoint drawPt = NSMakePoint (font->mCellWidth * x, font->mCellHeight * y);
		[nsString drawAtPoint: drawPt withAttributes:strFontAttribs]; // draw at offset position
		
		//generate glyph UVs in future texture
		
        Font::Glyph * pGlyph = &font->mGlyphs[c - firstChar];
        float charWidth = pGlyph->width;
		
		int col = x;
		int row = gridY - y - 1;
		
        pGlyph->upperLeft[0] = float(col * font->mCellWidth) / w;
        pGlyph->upperLeft[1] = float(row * font->mCellHeight) / h + vOffset;
		
        pGlyph->lowerLeft[0] = float(col * font->mCellWidth) / w;
        pGlyph->lowerLeft[1] = float((row + 1) * font->mCellHeight) / h + vOffset;
		
        pGlyph->lowerRight[0] = float((col * font->mCellWidth) + charWidth) / w;
        pGlyph->lowerRight[1] = float((row + 1) * font->mCellHeight) / h + vOffset;
		
        pGlyph->upperRight[0] = float((col * font->mCellWidth) + charWidth) / w;
        pGlyph->upperRight[1] = float(row * font->mCellHeight) / h + vOffset;
		
		//update grid positions
		
		if(++x >= gridX)
		{
			x = 0;
			++y;
		}
		
		//cleanup
		
		[nsString release];
	}
	
	NSBitmapImageRep * bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect (0.0f, 0.0f, w, h)];
	
	[image unlockFocus];
	
	//create texture
	
	w = (int)[bitmap pixelsWide];
	h = (int)[bitmap pixelsHigh];
	
	Image * sqImage = new Image(w, h, 1, Image::Int8, Image::Alpha);
	long pixelSize = [bitmap bytesPerRow] / w;
	unsigned char * bitmapData = [bitmap bitmapData];

    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
			unsigned char * pixelData = bitmapData + ((y * w) + x) * pixelSize;
			sqImage->getPixel(x, y)[0] = pixelData[3];
       }
    }
	
    // Only use GL_NEAREST filtering for the min and mag filter. Using anything
    // else will cause the font glyphs to be blurred. Using only GL_NEAREST
    // filtering ensures that edges of the font glyphs remain crisp and sharp.
	
	Resource::Texture * texture = new Resource::Texture(sqImage, RenderData::Image::Uncompressed, false);
	//texture->getRenderTexture()->setTexParameters(Render::ITexture::Filter::Linear, Render::ITexture::WrapMode::ClampToBorder, 1);
	font->mFontTexture = texture;

	[bitmap release];
	[image release];
	
	[autoreleasePool release];
	
    return font;
}
	
}//namespace Squirrel {
}//namespace GUI {
