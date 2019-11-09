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

#if !defined(GL_FONT_H)
#define GL_FONT_H

#include <Resource/Texture.h>
#include <Resource/Mesh.h>
#include "macros.h"

//Font class made from GLFont class of dhpoware.
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

namespace Squirrel {
namespace GUI {

#define VERTS_PER_CHAR	4
#define INDS_PER_CHAR	6

class FontRender
{
public:
	virtual void drawChar( Math::vec3 vertices[VERTS_PER_CHAR], Math::vec2 texcoords[VERTS_PER_CHAR] )	= 0;
};

class SQGUI_API Font
{

#ifdef _WIN32
	friend class WindowsFontGenerator;
#elif __APPLE__
	friend class MacFontGenerator;
#endif

public:
    enum Style
    {
        NORMAL,
        BOLD
    };

    struct Glyph
    {
        float width;
        Math::vec2 upperLeft;
        Math::vec2 lowerLeft;
        Math::vec2 upperRight;
        Math::vec2 lowerRight;
    };

    Font();
    ~Font();

    void drawText(float x, float y, const char *pszText);
	void drawChar(char c, float x, float y);

	void flush();

    float getCellHeight() const;
    float getCellWidth() const;
    const Glyph &getChar(char ch) const;
    float getCharHeight() const;
    float getCharWidthAvg() const;
    float getCharWidthMax() const;
    float getCharWidth(char ch) const;
    const char *getName() const;
    float getPointSize() const;
    float getStrWidth(const char *pszText) const;
    float getStrWidth(const char *pChar, size_t length) const;
	int getMaxFittingLength(const char *pszText, float bounds) const;

	Resource::Texture * getTexture();
	void setFontRender(FontRender * render);
	float getDepth();
	void setDepth(float depth);

private:
    Font(const Font &);
    Font &operator=(const Font &);

    void drawTextBegin();
    void drawTextEnd();

	void drawBatchOfChars();

	void generateTexCoords(float bmpWidth, float bmpHeight);

    static const int CHAR_FIRST = 32;
    static const int CHAR_LAST = 126;
    static const int TAB_SPACES = 4;
    static const int TOTAL_CHARS = 95;

    static const int MAX_CHARS_PER_BATCH = 512;
    static const int MAX_STR_SIZE = 1024;
    static const int MAX_VERTICES = MAX_CHARS_PER_BATCH * 4;

    std::string mName;
    float mPointSize;
    float mCellHeight;
    float mCellWidth;
    float mCharHeight;
    float mCharAvgWidth;
    float mCharMaxWidth;
    int mNumCharsToDraw;
    Glyph mGlyphs[TOTAL_CHARS];

	float mDepth;

	Resource::Texture * mFontTexture;
	Resource::Mesh * mTargetMesh;

	FontRender * mFontRender;

	int mCurrentVertex;
};

//-----------------------------------------------------------------------------

inline float Font::getDepth()
{ return mDepth; }

inline void Font::setDepth(float depth)
{ mDepth = depth; }

inline void Font::setFontRender(FontRender * render)
{ mFontRender = render; }

inline Resource::Texture * Font::getTexture()
{ return mFontTexture; }

inline float Font::getCellHeight() const
{ return mCellHeight; }

inline float Font::getCellWidth() const
{ return mCellWidth; }

inline const Font::Glyph &Font::getChar(char ch) const
{
    ASSERT(ch >= CHAR_FIRST && ch <= CHAR_LAST);
    return mGlyphs[ch - 32];
}

inline float Font::getCharHeight() const
{ return mCharHeight; }

inline float Font::getCharWidthAvg() const
{ return mCharAvgWidth; }

inline float Font::getCharWidthMax() const
{ return mCharMaxWidth; }

inline float Font::getCharWidth(char ch) const
{
	if(ch < CHAR_FIRST || ch > CHAR_LAST)
		return 0;
    return mGlyphs[ch - 32].width;
}

inline const char *Font::getName() const
{ return mName.c_str(); }

inline float Font::getPointSize() const
{ return mPointSize; }

//FontGenerator interface
//-----------------------------------------------------------------------------

class SQGUI_API FontGenerator
{
public:
	FontGenerator() {};
	virtual ~FontGenerator() {};

	virtual Font * create(const char * name, int size, Font::Style style) = 0;

	static FontGenerator * Active();
	void setAsActive();

protected:
	static FontGenerator * sFactory;
};

//-----------------------------------------------------------------------------

}//namespace Squirrel {
}//namespace GUI {

#endif