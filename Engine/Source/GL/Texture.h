#pragma once

#include <Common/types.h>
#include <Render/Image.h>
#include <Render/ITexture.h>
#include <Render/IRender.h>
#include <map>
#include "macros.h"

namespace Squirrel {

namespace GL { 

using namespace RenderData;
using namespace Render;

class SQOPENGL_API Texture:
	public ITexture
{
private: //nested types

	struct TexPixelFormat
	{
		TexPixelFormat():
			type(0),format(0),internalFormat(0),dataType(0)	{}
		TexPixelFormat(int type_,int format_,int internalFormat_,int dataType_):
			type(type_),format(format_),internalFormat(internalFormat_),dataType(dataType_)	{}
		int type;
		int format;
		int internalFormat;
		int dataType;
	};

	typedef std::pair<Image::Format,Image::DataType>	IMG_FORMAT;
	typedef	std::map<int,TexPixelFormat>				PIXEL_FORMAT_MAP;
	typedef	std::map<IMG_FORMAT,int>					IMG2TEX_FORMAT_MAP;

private: //members

	uint	mTexId;
	int		mUnit;

	bool	mMipmap;

	Image::Compression mCompression;
	TexPixelFormat mFormat;
	int		mTarget;//GL_TEXTURE_2D... (target)

	bool mShadow;

public://ctor/destr

	Texture();
	virtual ~Texture();

public: //methods

	int getTarget() { return mTarget; }
	uint getTexId() { return mTexId; }

	void bind(int unit = 0);
	void unbind();

	bool fill(Image * image, Image::Compression compress = Image::Uncompressed);
	bool fill(ITexture::PixelFormat format, tuple3i size);
	bool fillCube(PixelFormat format, int size);
	bool fillRect(PixelFormat format, int width, int height);
	bool fillFromScreen(PixelFormat format, int width, int height);

	Image * getImage();

	void setTexParameters(Filter filter, WrapMode wrap, float aniso);
	void enableShadow(bool enable);

	//implement IContextObject

	virtual void generate();
	virtual void destroy();

public: //static members

	const static int sCubeMapTargetsNum = 6;
	static int sCubeMapTarget[sCubeMapTargetsNum];

public: //static methods

	//returns compressed format from simple one
	static int	GetCompressionAnalog	(int internalFormat_);

	//target validation
	static bool	Validate2DTarget		(int target_);
	static bool ValidateCubeTarget		(int target_);
	static bool Validate3DTarget		(int target_);

	//tex units methods
	static void SetActiveUnit(int unit);
	static int  GetActiveUnit();

private: //static members

	//pixel format data
	static PIXEL_FORMAT_MAP		sPixFormatsMap;
	static IMG2TEX_FORMAT_MAP	sImg2TexFormatsMap;

private: //static methods

	//pixel format control
	static int				AddFormat(TexPixelFormat pixFormat);
	static TexPixelFormat	GetFormat(int type);
	static TexPixelFormat	GetFormat(Image::Format imgFormat, Image::DataType imgDataType);

	static int DetermineTarget(tuple3i size);

	static uint GetCompressedInternalFormat(Image * image, Image::Compression compress);

private: //methods

	bool fillFaces(Image * img, uint32 level);

};


} //namespace GL {

} //namespace Squirrel {
