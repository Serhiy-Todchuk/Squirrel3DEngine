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

#include "Font.h"
#include <Render/IRender.h>

namespace Squirrel {
namespace GUI {

using namespace RenderData;
using namespace Math;

FontGenerator * FontGenerator::sFactory = 0;

FontGenerator * FontGenerator::Active()
{
	return sFactory;
}

void FontGenerator::setAsActive()
{
	sFactory = this;
}

Font::Font()
{
    mName.clear();
    mPointSize = 0;
    mCellHeight = 0;
    mCellWidth = 0;
    mCharHeight = 0;
    mCharAvgWidth = 0;
    mCharMaxWidth = 0;
    mNumCharsToDraw = 0;
    mFontTexture = 0;
	mCurrentVertex = 0;

	mDepth = 0;

    memset(mGlyphs, 0, sizeof(mGlyphs));

	mTargetMesh = new Resource::Mesh();

	//init vertex buffer
	const int verticesNum = MAX_CHARS_PER_BATCH * 6;
	VertexBuffer * vb = mTargetMesh->createVertexBuffer(VT_TEXT, verticesNum);
	vb->setStorageType(IBuffer::stGPUDynamicMemory);

	//init index buffer
	const int indicesNum = MAX_CHARS_PER_BATCH * 6;
	IndexBuffer * ib = mTargetMesh->createIndexBuffer(indicesNum);
	ib->setPolyOri(IndexBuffer::poNone);
	ib->setPolyType(IndexBuffer::ptTriangles);
	ib->setStorageType(IBuffer::stGPUStaticMemory);
	for(int i = 0, j = 0, v = 0; i < MAX_CHARS_PER_BATCH; ++i)
	{
		ib->setIndex(j++, v + 0);	//   vertices:	|	  indices:
		ib->setIndex(j++, v + 3);	//	  0___3		|	0___1	    0
		ib->setIndex(j++, v + 1);	//	  |  /|		|	|  / 	   /|
		ib->setIndex(j++, v + 3);	//	  |/__|		|	|/	 	 /__|
		ib->setIndex(j++, v + 2);	//	  1   2		|	2	 	2   1
		ib->setIndex(j++, v + 1);	//				|
		v += 4;
	}

	mFontRender = NULL;

}

Font::~Font()
{
	DELETE_PTR(mFontTexture);
	DELETE_PTR(mTargetMesh);
}

void Font::drawChar(char c, float x, float y)
{
    //  1------4
    //  |      |            1 = (x, y)
    //  |      |            2 = (x, y + charHeight)
    //  |      |            3 = (x + charWidth, y + charHeight)
    //  |      |            4 = (x + charWidth, y)
    //  |      |
    //  |      |
    //  2------3
    //

    const Glyph &glyph = getChar(c);
    float charWidth = glyph.width;
    float charHeight = mCharHeight;

	VertexBuffer * vb = mTargetMesh->getVertexBuffer();

	vec3 pos[VERTS_PER_CHAR] = {
		vec3(x, y, mDepth),
		vec3(x, y + charHeight, mDepth),
		vec3(x + charWidth, y + charHeight, mDepth),
		vec3(x + charWidth, y, mDepth),
	};

	vec2 tex[VERTS_PER_CHAR] = {
		glyph.upperLeft,
		glyph.lowerLeft,
		glyph.lowerRight,
		glyph.upperRight
	};

	if(mFontRender == NULL)
	{
		for(int i = 0; i < VERTS_PER_CHAR; ++i)
		{
			int j = mCurrentVertex + i;
			vb->setComponent<VertexBuffer::vcPosition>( j, pos[i] );
			vb->setComponent<VertexBuffer::vcTexcoord>( j, tex[i] );
		}

		mCurrentVertex += VERTS_PER_CHAR;

		if (++mNumCharsToDraw == MAX_CHARS_PER_BATCH)
		{
			flush();
		}
	}
	else
	{
		mFontRender->drawChar(pos, tex);
	}
}

void Font::flush()
{
	drawTextEnd();
	//drawBatchOfChars();
	drawTextBegin();
}

void Font::drawTextBegin()
{
    mNumCharsToDraw = 0;
	mCurrentVertex = 0;
}

void Font::drawTextEnd()
{
    if (mNumCharsToDraw > 0)
        drawBatchOfChars();
}

void Font::drawBatchOfChars()
{
	Render::IRender * render = Render::IRender::GetActive();

	if(Render::ITexture::GetBoundTexture(0) != mFontTexture->getRenderTexture())
		mFontTexture->getRenderTexture()->bind();

	VertexBuffer * vb = mTargetMesh->getVertexBuffer();
	vb->update(0, mNumCharsToDraw * VERTS_PER_CHAR * vb->getVertexSize());

	render->setupVertexBuffer( vb );
	render->renderIndexBuffer( mTargetMesh->getIndexBuffer(), tuple2i(0, mNumCharsToDraw * 6) );
}

void Font::drawText(float x, float y, const char *pszText)
{
    char prevCh = 0;
    char ch = 0;
    float dx = x;
    float dy = y;
    float charHeight = getCellHeight();
    float whitespaceWidth = getChar(' ').width;

    while (*pszText != '\0')
    {
        prevCh = ch;
        ch = *pszText++;

        if (ch == ' ')
        {
            if (prevCh != '\r')
                dx += whitespaceWidth;
        }
        else if (ch == '\n' || ch == '\r')
        {
            dx = x;
            dy += charHeight;
        }
        else if (ch == '\t')
        {
            dx += whitespaceWidth * TAB_SPACES;
        }
        else if (ch >= CHAR_FIRST && ch <= CHAR_LAST)
        {
            drawChar(ch, dx, dy);
            dx += getChar(ch).width;
        }
    }
}

float Font::getStrWidth(const char *pszText) const
{
    if (!pszText)
        return 0;

    float width = 0;

    while (*pszText != '\0')
        width += getCharWidth(*pszText++);

    return width;
}

float Font::getStrWidth(const char *pChar, size_t length) const
{
    if (!pChar || length <= 0)
        return 0;

	length = Math::clamp<size_t>(length, 0, strlen(pChar));

    float width = 0;

    for (int i = 0; i < length; ++i)
        width += getCharWidth(pChar[i]);

    return width;
}

int Font::getMaxFittingLength(const char *pszText, float bounds) const
{
    if (!pszText || bounds <= 0)
        return 0;

	size_t length = strlen( pszText );
    float currWidth = 0;
	int index = 0;

	for(int i = 0; i < length; ++i)
	{
		if(currWidth < bounds)
		{
		  currWidth += getCharWidth(pszText[i]);
		  index++;
		}
		else break;
	}

    return (currWidth < bounds) ? index : index - 1;
}

void Font::generateTexCoords(float bmpWidth, float bmpHeight)
{
    Glyph *pGlyph = 0;
    int col = 0;
    int row = 0;
    float charWidth = 0;

    for (int c = 32; c < 127; ++c)
    {
        col = (c - 32) % 10;
        row = (c - 32) / 10;

        pGlyph = &mGlyphs[c - 32];
        charWidth = pGlyph->width;

        pGlyph->upperLeft[0] = (col * mCellWidth) / bmpWidth;
        pGlyph->upperLeft[1] = (row * mCellHeight) / bmpHeight;

        pGlyph->lowerLeft[0] = (col * mCellWidth) / bmpWidth;
        pGlyph->lowerLeft[1] = ((row + 1) * mCellHeight) / bmpHeight;

        pGlyph->lowerRight[0] = ((col * mCellWidth) + charWidth) / bmpWidth;
        pGlyph->lowerRight[1] = ((row + 1) * mCellHeight) / bmpHeight;

        pGlyph->upperRight[0] = ((col * mCellWidth) + charWidth) / bmpWidth;
        pGlyph->upperRight[1] = (row * mCellHeight) / bmpHeight;
    }
}

}//namespace Squirrel {
}//namespace GUI {
