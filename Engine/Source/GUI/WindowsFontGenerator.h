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

#pragma once

#include "Font.h"
#include <windows.h>

//WindowsFontGenerator class made from GLFont class of dhpoware.
//-----------------------------------------------------------------------------
// This GLFont class draws text as a bunch of textured quads. The GLFont class
// only stores the displayable ASCII characters in the range ASCII 32 to ASCII
// 126 inclusive. This range represents the displayable ASCII characters.
// ClearType text smoothing is used if the code is running under WinXP or
// higher. Otherwise standard text anti-aliasing is used.
//
// To use the GLFont class:
//  GLFont font;
//  font.create("Arial", 12, GLFont::NORMAL);
//  font.begin();
//  font.setColor(1.0f, 0.0f, 0.0f);
//  font.drawTextFormat(1, 1, "%s", "Hello, World!");
//  font.end();
//  font.destroy();
//-----------------------------------------------------------------------------

class Bitmap;

namespace Squirrel {
namespace GUI {

class SQGUI_API WindowsFontGenerator:
	public FontGenerator
{
public:
	WindowsFontGenerator();
	virtual ~WindowsFontGenerator();

	virtual Font * create(const char * name, int size, Font::Style style);

private:
    bool createTexture(const Bitmap &bitmap);
    bool createFontBitmap();
    bool extractFontMetrics();
    int nextPower2(int x) const;

    static int m_logPixelsY;
    static BYTE m_lfQuality;

    HFONT m_hFont;
    LOGFONT m_lf;
	Font * font;
};

}//namespace Squirrel {
}//namespace GUI {
