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

#include "WindowsFontGenerator.h"
#include <Render/Image.h>
#include <bitmap.h>

// Taken from: wingdi.h
#if !defined(CLEARTYPE_QUALITY)
#define CLEARTYPE_QUALITY 5
#endif

namespace Squirrel {
namespace GUI {

using namespace RenderData;

int WindowsFontGenerator::m_logPixelsY = 0;
BYTE WindowsFontGenerator::m_lfQuality = 0;

WindowsFontGenerator::WindowsFontGenerator()
{
	m_hFont = 0;

    if (!m_logPixelsY)
    {
        HWND hWndDesktop = GetDesktopWindow();
        HDC hDC = GetDCEx(hWndDesktop, 0, DCX_CACHE | DCX_WINDOW);

        if (hDC)
        {
            m_logPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
            ReleaseDC(hWndDesktop, hDC);
        }
    }

    if (!m_lfQuality)
    {
        DWORD dwVersion = GetVersion();
        DWORD dwMajorVersion = static_cast<DWORD>((LOBYTE(LOWORD(dwVersion))));
        DWORD dwMinorVersion = static_cast<DWORD>((HIBYTE(LOWORD(dwVersion))));

        // Windows XP and higher will support ClearType quality fonts.
        if (dwMajorVersion >= 6 || (dwMajorVersion == 5 && dwMinorVersion == 1))
            m_lfQuality = CLEARTYPE_QUALITY;
        else
            m_lfQuality = ANTIALIASED_QUALITY;
    }
}

WindowsFontGenerator::~WindowsFontGenerator()
{
}

Font * WindowsFontGenerator::create(const char *name, int size, Font::Style style)
{
	font = new Font();

    font->mName = name;
    font->mPointSize = size;

    m_lf.lfHeight = -MulDiv(size, m_logPixelsY, 72);
    m_lf.lfWidth = 0;
    m_lf.lfEscapement = 0;
    m_lf.lfOrientation = 0;
    m_lf.lfWeight = (style == Font::BOLD) ? FW_BOLD : FW_NORMAL;
    m_lf.lfItalic = FALSE;
    m_lf.lfUnderline = FALSE;
    m_lf.lfStrikeOut = FALSE;
    m_lf.lfCharSet = ANSI_CHARSET;
    m_lf.lfOutPrecision = OUT_STROKE_PRECIS;
    m_lf.lfClipPrecision = CLIP_STROKE_PRECIS;
    m_lf.lfQuality = m_lfQuality;
    m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    if (font->mName.length() < LF_FACESIZE)
        strcpy(m_lf.lfFaceName, name);
    else
        m_lf.lfFaceName[0] = '\0';

    HFONT hNewFont = CreateFontIndirect(&m_lf);

    if (!hNewFont)
        return false;

    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = 0;
    }

    m_hFont = hNewFont;

    if (!createFontBitmap())
    {
        //destroy();
		DeleteObject(m_hFont);
		DELETE_PTR( font );
        return NULL;
    }

	DeleteObject(m_hFont);

    return font;
}

bool WindowsFontGenerator::extractFontMetrics()
{
    HWND hWndDesktop = GetDesktopWindow();
    HDC hDC = GetDCEx(hWndDesktop, 0, DCX_CACHE | DCX_WINDOW);

    if (!hDC)
        return false;

    HFONT hPrevFont = reinterpret_cast<HFONT>(SelectObject(hDC, m_hFont));
    TEXTMETRIC tm;
    SIZE charSize = {0};
    char szString[2] = {0};
    char szName[128] = {0};

    GetTextFace(hDC, 128, szName);
    font->mName = szName;

    GetTextMetrics(hDC, &tm);
    font->mCharHeight = font->mCellHeight = tm.tmHeight + tm.tmExternalLeading;
    font->mCharMaxWidth = 0;
    font->mCharAvgWidth = 0;

    for (int c = 32; c < 127; ++c)
    {
        szString[0] = c;
        GetTextExtentPoint32(hDC, szString, 1, &charSize);

        if (charSize.cx > font->mCharMaxWidth)
            font->mCharMaxWidth = charSize.cx;

        font->mCharAvgWidth += charSize.cx;
        font->mGlyphs[c - 32].width = charSize.cx;
    }

    font->mCharAvgWidth /= Font::TOTAL_CHARS;
    font->mCellWidth = font->mCharMaxWidth + (font->mCharAvgWidth / 2);

    SelectObject(hDC, hPrevFont);
    ReleaseDC(hWndDesktop, hDC);
    return true;
}


bool WindowsFontGenerator::createTexture(const Bitmap &bitmap)
{
	Image * image = new Image(bitmap.width, bitmap.height, 1, Image::Int8, Image::Alpha);
	ASSERT( image );

	bitmap.copyBytesAlpha8Bit(image->getData());

    // Only use GL_NEAREST filtering for the min and mag filter. Using anything
    // else will cause the font glyphs to be blurred. Using only GL_NEAREST
    // filtering ensures that edges of the font glyphs remain crisp and sharp.

	Resource::Texture * texture = new Resource::Texture(image);
	font->mFontTexture = texture;

    return true;
}

bool WindowsFontGenerator::createFontBitmap()
{
    // The font is drawn as a 10 x 10 grid of characters.

    if (!extractFontMetrics())
        return false;

    int w = 10 * font->mCellWidth;
    int h = 10 * font->mCellHeight;
    Bitmap bitmap;

	if (!bitmap.create(	Math::getNextPowerOfTwo(10 * font->mCellWidth), 
						Math::getNextPowerOfTwo(10 * font->mCellHeight) ))
        return false;

    int x = 0;
    int y = 0;
    int ch = 32;
    HFONT hPrevFont = 0;
    COLORREF prevColor = 0;
    RECT rc = {0, 0, bitmap.width, bitmap.height};

    bitmap.selectObject();
    hPrevFont = reinterpret_cast<HFONT>(SelectObject(bitmap.dc, m_hFont));
    prevColor = SetTextColor(bitmap.dc, RGB(255,255,255));
    SetBkMode(bitmap.dc, TRANSPARENT);
    FillRect(bitmap.dc, &rc, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

    for (int i = 0; i < 10; ++i)
    {
        y = font->mCellHeight * i;

        for (int j = 0; j < 10; ++j)
        {
            x = font->mCellWidth * j;

            if (ch > 31 && ch < 127)
                TextOut(bitmap.dc, x, y, reinterpret_cast<LPCSTR>(&ch), 1);

            ++ch;
        }
    }

    SetTextColor(bitmap.dc, prevColor);
    SelectObject(bitmap.dc, hPrevFont);
    bitmap.deselectObject();

	font->generateTexCoords((float)bitmap.width, (float)bitmap.height);
    return createTexture(bitmap);
}

}//namespace Squirrel {
}//namespace GUI {
