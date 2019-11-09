#include "ImageLoader.h"
#include <Common/Log.h>
#include <Common/DynamicLibrary.h>
#include <IL/il.h>

#ifdef __APPLE__
#include <Render/Mac/MacImageLoader.h>
#endif

namespace Squirrel {

typedef	ILenum    (ILAPIENTRY	*LPilGetError)		(void);
typedef	void      (ILAPIENTRY	*LPilInit)			(void);
typedef	ILboolean (ILAPIENTRY	*LPilEnable)		(ILenum Mode);
typedef	void	  (ILAPIENTRY	*LPilGenImages)		(ILsizei Num, ILuint *Images);
typedef	void	  (ILAPIENTRY	*LPilBindImage)		(ILuint Image);
typedef	ILboolean (ILAPIENTRY	*LPilLoadL)			(ILenum Type, const void *Lump, ILuint Size);
typedef	ILboolean (ILAPIENTRY	*LPilConvertImage)	(ILenum DestFormat, ILenum DestType);
typedef	void      (ILAPIENTRY	*LPilDeleteImages)	(ILsizei Num, const ILuint *Images);
typedef	ILuint    (ILAPIENTRY	*LPilCopyPixels)	(ILuint XOff, ILuint YOff, ILuint ZOff, ILuint Width, ILuint Height, ILuint Depth, ILenum Format, ILenum Type, void *Data);
typedef	ILint     (ILAPIENTRY	*LPilGetInteger)	(ILenum Mode);

LPilGetError		_ilGetError		= NULL;
LPilInit			_ilInit			= NULL;
LPilEnable			_ilEnable		= NULL;
LPilGenImages		_ilGenImages	= NULL;
LPilBindImage		_ilBindImage	= NULL;
LPilLoadL			_ilLoadL		= NULL;
LPilConvertImage	_ilConvertImage	= NULL;
LPilDeleteImages	_ilDeleteImages	= NULL;
LPilCopyPixels		_ilCopyPixels	= NULL;
LPilGetInteger		_ilGetInteger	= NULL;

DynamicLibrary sOpenILModule;

namespace Resource { 

using namespace RenderData;
/*
char CubeMapTargetSTR[6][4]=	
{ 
	"_nx","_px",
	"_ny","_py",
	"_nz","_pz"
};

char CubeMapTargetSTR2[6][9]=	
{
	"_right", "_left",
	"_down",   "_up",
	"_forward", "_back"
};
*/
bool CheckILError(const char * msg)
{
	ILenum errCode = _ilGetError();
	if(errCode != IL_NO_ERROR)
	{
		//const char * str = iluErrorString(ilGetError());
		char str[32];
		sprintf(str, "%04X", errCode);
		Log::Instance().streamError(msg) << "IL error code: " << str;
		Log::Instance().flush();
		return true;
	}
	return false;
}

ImageLoader::ImageLoader(void)
{
	mInitialized = false;
	mForceBGR2RGB = true;
}

ImageLoader::~ImageLoader(void)
{
}

void ImageLoader::init()
{
	if(!sOpenILModule.isOpened())
	{
		if(!sOpenILModule.open("DevIL"))
		{
			Log::Instance().error("ImageLoader::init", "Failed to initialize DevIL module!");
			return;
		}

		_ilGetError		= (LPilGetError)		sOpenILModule.getSymbol("ilGetError");
		_ilInit			= (LPilInit)			sOpenILModule.getSymbol("ilInit");
		_ilEnable		= (LPilEnable)			sOpenILModule.getSymbol("ilEnable");
		_ilGenImages	= (LPilGenImages)		sOpenILModule.getSymbol("ilGenImages");
		_ilBindImage	= (LPilBindImage)		sOpenILModule.getSymbol("ilBindImage");
		_ilLoadL		= (LPilLoadL)			sOpenILModule.getSymbol("ilLoadL");
		_ilConvertImage	= (LPilConvertImage)	sOpenILModule.getSymbol("ilConvertImage");
		_ilDeleteImages	= (LPilDeleteImages)	sOpenILModule.getSymbol("ilDeleteImages");
		_ilCopyPixels	= (LPilCopyPixels)		sOpenILModule.getSymbol("ilCopyPixels");
		_ilGetInteger	= (LPilGetInteger)		sOpenILModule.getSymbol("ilGetInteger");

		if(_ilInit == NULL)
		{
			Log::Instance().error("ImageLoader::init", "Failed to obtain appropriate DevIL methods!");
			return;
		}
	}

	if(!mInitialized)
	{
		_ilInit();
		_ilEnable(IL_ORIGIN_SET);
		_ilEnable(IL_CONV_PAL);

		mInitialized = true;
	}
}

Image* ImageLoader::loadImage(std::string fileName)
{
	Data * data = new Data( fileName.c_str() );
	if(data)
	{
		return loadImage(data);
	}
	return NULL;
}

Image* ImageLoader::loadImage(Data * data)
{
#ifdef __APPLE__
	return MacImageLoader::LoadImage(data);
#endif
	
	init();//save to call

	if(!mInitialized)
	{
		return NULL;
	}

	ILuint handle, w, h, d, format, type;

	_ilGenImages(1, &handle);
	_ilBindImage(handle);

	ILboolean loaded = _ilLoadL(IL_TYPE_UNKNOWN, data->getData(), data->getLength());
	if (loaded == IL_FALSE)
	{
		CheckILError("ilLoadImage failed!");
		return NULL; /* error encountered during loading */
	}

	//get image parameters
	w		= _ilGetInteger(IL_IMAGE_WIDTH); // getting image width
	h		= _ilGetInteger(IL_IMAGE_HEIGHT); // and height
	d		= _ilGetInteger(IL_IMAGE_DEPTH); // and depth
	format	= _ilGetInteger(IL_IMAGE_FORMAT);
	type	= _ilGetInteger(IL_IMAGE_TYPE);

	if(mForceBGR2RGB)
	{
		Image::Format newFormat = Image::UnknownFormat;
		switch(format)
		{
		case Image::BGR : newFormat = Image::RGB ; break;
		case Image::BGRA: newFormat = Image::RGBA; break;
		}
		if(newFormat != Image::UnknownFormat)
		{
			format = newFormat;
			if( !_ilConvertImage( format, type ) )
			{
				CheckILError("ilConvertImage failed!");
			}
		}
	}

	//make type unsigned
	switch(type)
	{
	case IL_BYTE:
	case IL_SHORT:
	case IL_INT:
		++type;
	};

	Image* image = new Image(w, h, d, (Image::DataType)type, (Image::Format)format); 

	/* finally get the image data */
	_ilCopyPixels(0, 0, 0, w, h, d, format, type, image->getData());

	if(CheckILError("ilCopyPixels failed!"))
	{
		delete image;
		_ilDeleteImages(1, &handle);
		return NULL;
	}

	/* clean */
	_ilDeleteImages(1, &handle);
	
	return image;
}


}//namespace Resource { 

}//namespace Squirrel {