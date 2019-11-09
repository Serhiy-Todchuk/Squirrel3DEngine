#include "ITexture.h"

namespace Squirrel {

namespace Render { 

ITexture *						ITexture::sUnits[sMaxUnits] = {0, 0, 0, 0, 0, 0, 0, 0};
int								ITexture::sActiveUnit = 0;
ITexture::IMG2TEX_FORMAT_MAP	ITexture::sImg2TexFormatsMap;

ITexture::PixelFormat	ITexture::GetFormat(Image::Format imgFormat, Image::DataType imgDataType)
{
	if(sImg2TexFormatsMap.size() == 0)
	{
		sImg2TexFormatsMap[ IMG_FORMAT(Image::Alpha,			Image::Int8) ] = ITexture::pfAlphaByte;
		sImg2TexFormatsMap[ IMG_FORMAT(Image::Luminance,		Image::Int8) ] = ITexture::pfColor1Byte;
		sImg2TexFormatsMap[ IMG_FORMAT(Image::LuminanceAlpha,	Image::Int8) ] = ITexture::pfColor2Byte;
		sImg2TexFormatsMap[ IMG_FORMAT(Image::RGB,				Image::Int8) ] = ITexture::pfColor3Byte;
		sImg2TexFormatsMap[ IMG_FORMAT(Image::RGBA,				Image::Int8) ] = ITexture::pfColor4Byte;
	}
	IMG2TEX_FORMAT_MAP::const_iterator ftIter = sImg2TexFormatsMap.find( IMG_FORMAT(imgFormat, imgDataType) );
	if(ftIter != sImg2TexFormatsMap.end())
	{
		return ftIter->second;
	}

	return ITexture::pfUnknown;
}

} //namespace Render {

} //namespace Squirrel {
