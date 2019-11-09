#pragma once

#include "IContextObject.h"
#include <Common/types.h>
#include <Common/tuple.h>
#include <Common/DataMap.h>
#include <Render/Image.h>
#include <map>

namespace Squirrel {

namespace Render { 

using namespace RenderData;

class SQRENDER_API ITexture:
	public IContextObject
{
public: //nested types

	enum CubeMapFace
	{
		cmfPosX = 0,
		cmfNegX,
		cmfPosY,
		cmfNegY,
		cmfPosZ,
		cmfNegZ,
		cmfNum
	};

	enum PixelFormat
	{
		pfUnknown = 0,
		pfAlphaByte = 1,
		pfColor1Byte,
		pfColor2Byte,
		pfColor3Byte,
		pfColor4Byte,
		pfColor1Half,
		pfColor2Half,
		pfColor3Half,
		pfColor4Half,
		pfColor1Float,
		pfColor2Float,
		pfColor3Float,
		pfColor4Float,
		pfDepth16,
		pfDepth24,
		pfDepth32
	};

	enum Filter
	{
		Nearest = 1,
		Linear,
		Trilinear
	};

	enum WrapMode
	{
		Clamp = 1,
		Repeat,
		ClampToBorder,
		ClampToEdge
	};

private: //nested types

	typedef std::pair<Image::Format, Image::DataType>	IMG_FORMAT;
	typedef	Trie<IMG_FORMAT, PixelFormat>			IMG2TEX_FORMAT_MAP;

public://ctor/destr

	ITexture() {};
	virtual ~ITexture() {};

public: //methods

	virtual void setTexParameters(Filter filter, WrapMode wrap, float aniso) = 0;
	virtual void bind(int unit = 0)						= 0;
	virtual void unbind()								= 0;
	virtual bool fill(Image * image, Image::Compression compress = Image::Uncompressed)	= 0;
	virtual bool fill(PixelFormat format, tuple3i size)	= 0;
	virtual bool fillRect(PixelFormat format, int width, int height) = 0;
	virtual bool fillFromScreen(PixelFormat format, int width, int height) = 0;
	virtual bool fillCube(PixelFormat format, int size)	= 0;
	virtual Image * getImage()							= 0;
	virtual void enableShadow(bool enable)				= 0;

	static PixelFormat	GetFormat(Image::Format imgFormat, Image::DataType imgDataType);

	static ITexture * GetBoundTexture(int unit) { return sUnits[unit]; }

	static void Unbind(int unit) { 
		ITexture * tex = GetBoundTexture(unit);
		if(tex != NULL)
			tex->unbind();
	}

public: //static members

	const static int sMaxUnits = 32;

protected: //static members

	//pixel format data
	static IMG2TEX_FORMAT_MAP	sImg2TexFormatsMap;

	//tex units members
	static ITexture * sUnits[sMaxUnits];
	static int sActiveUnit;
};


} //namespace Render {

} //namespace Squirrel {
