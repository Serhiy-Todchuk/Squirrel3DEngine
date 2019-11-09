#include "Texture.h"
#include <Render/Image.h>
#include "Render.h"
#include "Utils.h"
# include <GL/glee.h>

namespace Squirrel {

namespace GL { 

Texture::PIXEL_FORMAT_MAP		Texture::sPixFormatsMap;
Texture::IMG2TEX_FORMAT_MAP		Texture::sImg2TexFormatsMap;

int	Texture::sCubeMapTarget[sCubeMapTargetsNum]=	
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

int	Texture::AddFormat(TexPixelFormat pixFormat)
{
	if(pixFormat.type<1) return pixFormat.type;
	sPixFormatsMap[pixFormat.type]=pixFormat;
	return pixFormat.type;
}

Texture::TexPixelFormat	Texture::GetFormat(int type)
{
	if(sPixFormatsMap.empty())
	{
		//GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA are deprecated in opengl 3, use R and RG channels instead
#ifndef SUPPORT_GL3
		
		 AddFormat( TexPixelFormat( ITexture::pfAlphaByte,	GL_ALPHA,			GL_ALPHA8,									GL_UNSIGNED_BYTE ) );
		 AddFormat( TexPixelFormat( ITexture::pfColor1Byte,	GL_LUMINANCE,		GL_LUMINANCE8,								GL_UNSIGNED_BYTE ) );
		 AddFormat( TexPixelFormat( ITexture::pfColor2Byte,	GL_LUMINANCE_ALPHA,	GL_LUMINANCE8_ALPHA8,						GL_UNSIGNED_BYTE ) );

		 AddFormat( TexPixelFormat( ITexture::pfColor1Half,	GL_ALPHA,			Utils::fpAlphaFormatWithPrecision(16),	GL_UNSIGNED_BYTE ) );
		 AddFormat( TexPixelFormat( ITexture::pfColor2Half,	GL_LUMINANCE_ALPHA,	Utils::fpRGFormatWithPrecision(16),		GL_UNSIGNED_BYTE ) );

		 AddFormat( TexPixelFormat( ITexture::pfColor1Float,	GL_ALPHA,			Utils::fpAlphaFormatWithPrecision(32),	GL_UNSIGNED_BYTE ) );
		 AddFormat( TexPixelFormat( ITexture::pfColor2Float,	GL_LUMINANCE_ALPHA,	Utils::fpRGFormatWithPrecision(32),		GL_UNSIGNED_BYTE ) );
#else
		AddFormat( TexPixelFormat( ITexture::pfAlphaByte,	GL_RED,		GL_R8,			GL_UNSIGNED_BYTE ) );
		AddFormat( TexPixelFormat( ITexture::pfColor1Byte,	GL_RED,		GL_R8,			GL_UNSIGNED_BYTE ) );
		AddFormat( TexPixelFormat( ITexture::pfColor2Byte,	GL_RG,		GL_RG8,			GL_UNSIGNED_BYTE ) );
	
		AddFormat( TexPixelFormat( ITexture::pfColor1Half,	GL_RED,		GL_R16F,		GL_UNSIGNED_BYTE ) );
		AddFormat( TexPixelFormat( ITexture::pfColor2Half,	GL_RG,		GL_RG16F,		GL_UNSIGNED_BYTE ) );
		
		AddFormat( TexPixelFormat( ITexture::pfColor1Float,	GL_RED,		GL_R32F,		GL_UNSIGNED_BYTE ) );
		AddFormat( TexPixelFormat( ITexture::pfColor2Float,	GL_RG,		GL_RG32F,		GL_UNSIGNED_BYTE ) );
#endif
		
		AddFormat( TexPixelFormat( ITexture::pfColor3Byte,	GL_RGB,		GL_RGB8,		GL_UNSIGNED_BYTE ) );
		AddFormat( TexPixelFormat( ITexture::pfColor4Byte,	GL_RGBA,	GL_RGBA8,		GL_UNSIGNED_BYTE ) );
		
		AddFormat( TexPixelFormat( ITexture::pfColor3Half,	GL_RGB,		Utils::fpRGBFormatWithPrecision(16),	GL_UNSIGNED_BYTE ));
		AddFormat( TexPixelFormat( ITexture::pfColor4Half,	GL_RGBA,	Utils::fpRGBAFormatWithPrecision(16),	GL_UNSIGNED_BYTE ));
		AddFormat( TexPixelFormat( ITexture::pfColor3Float,	GL_RGB,		Utils::fpRGBFormatWithPrecision(32),	GL_UNSIGNED_BYTE ));
		AddFormat( TexPixelFormat( ITexture::pfColor4Float,	GL_RGBA,	Utils::fpRGBAFormatWithPrecision(32),	GL_UNSIGNED_BYTE ));
		
		AddFormat( TexPixelFormat( ITexture::pfDepth16,		GL_DEPTH_COMPONENT,	GL_DEPTH_COMPONENT16,	GL_UNSIGNED_INT ) );
		AddFormat( TexPixelFormat( ITexture::pfDepth24,		GL_DEPTH_COMPONENT,	GL_DEPTH_COMPONENT24,	GL_UNSIGNED_INT ) );
		AddFormat( TexPixelFormat( ITexture::pfDepth32,		GL_DEPTH_COMPONENT,	GL_DEPTH_COMPONENT32,	GL_UNSIGNED_INT ) );
	}

	PIXEL_FORMAT_MAP::const_iterator ftIter = sPixFormatsMap.find( type );
	if(ftIter != sPixFormatsMap.end())
	{
		return ftIter->second;
	}

	return TexPixelFormat(0,0,0,0);
}

Texture::TexPixelFormat	Texture::GetFormat(Image::Format imgFormat, Image::DataType imgDataType)
{
	ITexture::PixelFormat pf = ITexture::GetFormat( imgFormat, imgDataType );
	if(pf != ITexture::pfUnknown)
	{
		return GetFormat((int)pf);
	}

	return TexPixelFormat(0,0,0,0);
}

void Texture::SetActiveUnit(int unit)
{
	if(sActiveUnit != unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		sActiveUnit = unit;
	}
}

int Texture::GetActiveUnit()
{
	return sActiveUnit;
}

bool Texture::Validate2DTarget(int target)
{
	if((target==GL_TEXTURE_2D) || (target==GL_TEXTURE_RECTANGLE_ARB))
		return true;
	for(unsigned int i = 0; i<sCubeMapTargetsNum; ++i)
		if(target == sCubeMapTarget[i])
			return true;
	return false;
}

bool Texture::ValidateCubeTarget(int target_)
{
	if(target_==GL_TEXTURE_CUBE_MAP_ARB)
		return true;
	return false;
}

bool Texture::Validate3DTarget(int target_)
{
	if((target_==GL_TEXTURE_2D_ARRAY_EXT) || (target_==GL_TEXTURE_3D_EXT))
		return true;
	return false;
}

int Texture::GetCompressionAnalog(int internalFormat_)
{
	//if(Utils::ext_supports.is_ARB_texture_compression == false) return internalFormat_;
	switch(internalFormat_)
	{
	case GL_RGB:	return GL_COMPRESSED_RGB_ARB;	break;
	case GL_RGBA:	return GL_COMPRESSED_RGBA_ARB;	break;
	case GL_RGB8:	return GL_COMPRESSED_RGB_ARB;	break;
	case GL_RGBA8:	return GL_COMPRESSED_RGBA_ARB;	break;
	case GL_ALPHA8:	return GL_COMPRESSED_ALPHA_ARB;	break;
	case GL_LUMINANCE8:	return GL_COMPRESSED_LUMINANCE_ARB;	break;
	case GL_LUMINANCE8_ALPHA8:	return GL_COMPRESSED_LUMINANCE_ALPHA_ARB;	break;
	}
	return internalFormat_;
}


int Texture::DetermineTarget(tuple3i size)
{
	if(size.x <= 0 || size.y <= 0) return 0;
	if(size.z == 1)
	{
		return GL_TEXTURE_2D;
	}
	else if(size.z > 1)
	{
		return GL_TEXTURE_3D;
	}
	return 0;
}

Texture::Texture(): 
	mTexId(0), mShadow(false), mTarget(0)
{
}

Texture::~Texture()
{
	setPool(NULL);
}

void Texture::generate()
{
	glGenTextures(1, &mTexId);
	CHECK_GL_ERROR;
}

void Texture::destroy()
{
	if(mTexId != 0)
	{
		glDeleteTextures(1, &mTexId);
		mTexId = 0;
	}
}

void Texture::setTexParameters(Filter filter, WrapMode wrap, float aniso)
{
	glTexParameterf ( mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, Math::maxValue(1.0f, aniso) );
	CHECK_GL_ERROR;
	
	switch(filter)
	{
	case Nearest:
		glTexParameteri ( mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri ( mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		break;
	case Linear:
		glTexParameteri ( mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri ( mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	case Trilinear:
		glTexParameteri ( mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri ( mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		break;
	default:
		glTexParameteri ( mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri ( mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	CHECK_GL_ERROR;
	
	int glWrap = GL_CLAMP;
	switch(wrap)
	{
	case Clamp:			glWrap = GL_CLAMP;	break;
	case Repeat:		glWrap = GL_REPEAT;	break;
	case ClampToBorder: glWrap = GL_CLAMP_TO_BORDER; break;
	case ClampToEdge:	glWrap = GL_CLAMP_TO_EDGE; break;
	default:			glWrap = GL_CLAMP; break;
	}

	glTexParameteri ( mTarget, GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri ( mTarget, GL_TEXTURE_WRAP_T, glWrap);
	glTexParameteri ( mTarget, GL_TEXTURE_WRAP_R, glWrap);
	
	CHECK_GL_ERROR;
}

void Texture::bind(int unit)
{
	mUnit = unit;
	SetActiveUnit(unit);
	if(sUnits[unit] != this && mTarget != 0)
	{
		//if(target != GL_TEXTURE_2D_ARRAY_EXT) 
		//	glEnable(target);
		glBindTexture(mTarget, mTexId);
		sUnits[unit] = this;
	}
}

void Texture::unbind()
{
	if(sUnits[mUnit] == this)
	{
		SetActiveUnit(mUnit);
		glBindTexture(mTarget, 0);
		sUnits[mUnit] = 0;
	}
	//if(target != GL_TEXTURE_2D_ARRAY_EXT) 
	//	glDisable(target);
}

uint Texture::GetCompressedInternalFormat(Image * image, Image::Compression compress)
{
	bool is2DTexture = image->getWidth() > 1 && image->getHeight() > 1 && image->getDepth() == 1;
	switch(compress)
	{
	case Image::DXT1:
		if(is2DTexture && image->getDataType() == Image::Int8)
		{
			if(image->getComponentsNum() == 3)
				return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			else if(image->getComponentsNum() == 4)
				return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		}
		break;
	case Image::DXT3:
		if(is2DTexture && image->getDataType() == Image::Int8)
		{
			if(image->getComponentsNum() == 4)
				return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		}
		break;
	case Image::DXT5:
		if(is2DTexture && image->getDataType() == Image::Int8)
		{
			if(image->getComponentsNum() == 4)
				return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		}
		break;
	case Image::LATC:
		if(is2DTexture && image->getDataType() == Image::Int8)
		{
			if(image->getComponentsNum() == 1)
				return GL_COMPRESSED_LUMINANCE_LATC1_EXT;
			else if(image->getComponentsNum() == 2)
				return GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
		}
		break;
	case Image::LATCSigned:
		if(is2DTexture && image->getDataType() == Image::Int8)
		{
			if(image->getComponentsNum() == 1)
				return GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT;
			else if(image->getComponentsNum() == 2)
				return GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT;
		}
		break;
	case Image::PVR2BPP:
	case Image::PVR4BPP:
			ASSERT(false);//TODO: implement
	case Image::Uncompressed:
			return 0;
	//#ifdef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
	}
	return 0;
}

bool Texture::fill(Image * image, Image::Compression compress)
{
	//make sure that texture is bound
	//bind(GetActiveUnit());

	//TODO: (    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment)

	mFormat = GetFormat(image->getFormat(), image->getDataType());
	if(mFormat.type == 0) return false;

	if(	image->getFacesNum() == sCubeMapTargetsNum && 
		image->getWidth() == image->getHeight() && 
		image->getDepth() == 1 )
	{
		mTarget = GL_TEXTURE_CUBE_MAP;
	}
	else
	{
		mTarget	= DetermineTarget(tuple3i(image->getWidth(), image->getHeight(), image->getDepth()));
	}
	if(mTarget == 0) return false;

	//try to force compress
	if(image->getCompression() == Image::Uncompressed && compress != Image::Uncompressed)
	{
		uint compressedInternalFormat = GetCompressedInternalFormat(image, compress);
		if(compressedInternalFormat != 0)
			mFormat.internalFormat = compressedInternalFormat;
	}
	else if(image->getCompression() != Image::Uncompressed)
	{
		uint compressedInternalFormat = GetCompressedInternalFormat(image, image->getCompression());
		if(compressedInternalFormat != 0)
			mFormat.internalFormat = compressedInternalFormat;
	}

	//make sure that texture is bound
	bind(GetActiveUnit());
	CHECK_GL_ERROR;

	//set parameters
	setTexParameters(image->getLevelsNum() > 1 ? Trilinear : Linear, Repeat, 8);
	glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, image->getLevelsNum() - 1);
	CHECK_GL_ERROR;
	
	if(image->getLevelsNum() > 1) mMipmap = true;

	//fill texture data
	for(uint32 i = 0; i < image->getLevelsNum(); ++i)
	{
		fillFaces(image, i);
	}

	//check if texture has been forcibly compressed
	GLint compressed = 0;
	glGetTexLevelParameteriv(mTarget == GL_TEXTURE_CUBE_MAP ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : mTarget, 
							 0, GL_TEXTURE_COMPRESSED, &compressed);
	bool hasBeenCompressed = image->getCompression() == Image::Uncompressed && compressed;
	if(hasBeenCompressed)
	{
		mCompression = compress;
	}
	else
	{
		mCompression = image->getCompression();
	}

	Utils::CheckGLError("Texture::fill failed!");

	return true;
}

bool Texture::fillFaces(Image * img, uint32 level)
{
	if(level >= img->getLevelsNum()) return false;

	uint32 w = img->getWidth();
	uint32 h = img->getHeight();
	uint32 d = img->getDepth();

	bool compressed = img->getCompression() != Image::Uncompressed;
	bool cubemap =	img->getFacesNum() == sCubeMapTargetsNum && 
					mTarget == GL_TEXTURE_CUBE_MAP && 
					w == h && d == 1;

	for(uint i = 0; i < img->getFacesNum(); ++i)
	{
		const Image::Level& data = img->getLevel(level, i);

		if(h == 1 && d == 1)//1D texture
		{
			//compressed is not supported yet
			glTexImage1D(	mTarget, level, mFormat.internalFormat, data.dimensions.x, 
							0, mFormat.format, mFormat.dataType, data.data);
		}
		else if(d == 1)//2D texture
		{
			int target = mTarget;

			if(cubemap)
			{
				target = sCubeMapTarget[i];
			}

			if(compressed)
				glCompressedTexImage2D(	target, level, mFormat.internalFormat, data.dimensions.x, 
										data.dimensions.y, 0, data.size, data.data);
			else
				glTexImage2D(	target, level, mFormat.internalFormat, data.dimensions.x, 
								data.dimensions.y, 0, mFormat.format, mFormat.dataType, data.data);
		}
		else//3D texture
		{
			//compressed is not supported yet
			glTexImage3D(	mTarget, level, mFormat.internalFormat, data.dimensions.x, data.dimensions.y,
							data.dimensions.z, 0, mFormat.format, mFormat.dataType, data.data);
		}
		Utils::CheckGLError("Texture::fillFace failed!");
	}

	return true;
}

bool Texture::fill(ITexture::PixelFormat format, tuple3i size)
{
	mTarget = DetermineTarget(size);

	//make sure that texture is bound
	bind(GetActiveUnit());

	mFormat = GetFormat(format);
	if(mFormat.type == 0) return false;

	setTexParameters(Linear, ClampToEdge, 0);
	glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, 0);

	glTexImage2D(mTarget, 0, mFormat.internalFormat, size.x, 
		size.y, 0, mFormat.format, mFormat.dataType, NULL);

	Utils::CheckGLError("Texture::fill(empty) failed!");

	return true;
}

bool Texture::fillFromScreen(ITexture::PixelFormat format, int width, int height)
{
	//mTarget = Utils::getTexRectTarget();
	if(mTarget <= 0)
		mTarget = GL_TEXTURE_2D;

	//make sure that texture is bound
	bind(GetActiveUnit());

	mFormat = GetFormat(format);
	if(mFormat.type == 0) return false;

	setTexParameters(Linear, ClampToEdge, 0);
	glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, 0);
	
	glCopyTexImage2D(mTarget, 0, mFormat.internalFormat, 0, 0, width, height, 0);

	Utils::CheckGLError("Texture::fillFromScreen() failed!");
	
	return true;
}

bool Texture::fillRect(ITexture::PixelFormat format, int width, int height)
{
	mTarget = Utils::getTexRectTarget();
	if(mTarget <= 0)
		mTarget = GL_TEXTURE_2D;

	//make sure that texture is bound
	bind(GetActiveUnit());
	
	mFormat = GetFormat(format);
	if(mFormat.type == 0) return false;
	
	setTexParameters(Nearest, ClampToEdge, 0);
	glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, 0);
	
	glTexImage2D(mTarget, 0, mFormat.internalFormat, width,
				 height, 0, mFormat.format, mFormat.dataType, NULL);
	Utils::CheckGLError("Texture::fillRect(empty) failed!");
	
	return true;
}


bool Texture::fillCube(ITexture::PixelFormat format, int size)
{
	mTarget = GL_TEXTURE_CUBE_MAP;

	//make sure that texture is bound
	bind(GetActiveUnit());

	mFormat = GetFormat(format);
	if(mFormat.type == 0) return false;

	setTexParameters(Linear, ClampToEdge, 0);
	glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, 0);

	for(uint i = 0; i < sCubeMapTargetsNum; ++i)
	{
		glTexImage2D( sCubeMapTarget[i], 0, mFormat.internalFormat, size, 
			size, 0, mFormat.format, mFormat.dataType, NULL);
	}

	Utils::CheckGLError("Texture::fillCube(empty) failed!");

	return true;
}

void Texture::enableShadow(bool enable)
{
	if(enable && !mShadow)
	{
		// This is to allow usage of shadow2DProj function in the shader
		glTexParameteri( mTarget , GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE ); 
		glTexParameteri( mTarget, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//glTexParameteri( mTarget, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	}
	if(!enable && mShadow)
	{
		glTexParameteri( mTarget , GL_TEXTURE_COMPARE_MODE, GL_NONE ); 
	}

	mShadow = enable;
}

Image * Texture::getImage()
{
	int w, h, d;

	bool isCubemap = mTarget == GL_TEXTURE_CUBE_MAP;
	int faces = isCubemap ? 6 : 1;
	int subTarget = isCubemap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : mTarget; 

	//make sure that texture is bound
	bind(GetActiveUnit());
							 
	glGetTexLevelParameteriv(subTarget, 0, GL_TEXTURE_WIDTH,	&w);
	glGetTexLevelParameteriv(subTarget, 0, GL_TEXTURE_HEIGHT,	&h);
	glGetTexLevelParameteriv(subTarget, 0, GL_TEXTURE_DEPTH,	&d);

	int levels = 0;
	glGetTexParameteriv(mTarget, GL_TEXTURE_MAX_LEVEL, &levels);
	++levels;

	int compressed = 0;
	glGetTexLevelParameteriv(subTarget, 0, GL_TEXTURE_COMPRESSED, &compressed);

	Image::Format format = Image::UnknownFormat;
	Image::DataType dataType = Image::UnknownDataType;

	/*
	//get image format and data type from texture format
	IMG2TEX_FORMAT_MAP::iterator img2TexFormatIt = sImg2TexFormatsMap.begin();
	for(img2TexFormatIt; img2TexFormatIt != sImg2TexFormatsMap.end(); ++img2TexFormatIt)
	{
		if(img2TexFormatIt->second == mFormat.type)
		{
			format		= img2TexFormatIt->first.first;
			dataType	= img2TexFormatIt->first.second;
			break;
		}
	}
	*/

	//get image format and data type from texture format
	switch(mFormat.type)
	{
	case ITexture::pfAlphaByte:		dataType = Image::Int8;		format = Image::Alpha;			break;
	case ITexture::pfColor1Byte:	dataType = Image::Int8;		format = Image::Luminance;		break;
	case ITexture::pfColor2Byte:	dataType = Image::Int8;		format = Image::LuminanceAlpha;	break;
	case ITexture::pfColor3Byte:	dataType = Image::Int8;		format = Image::RGB;			break;
	case ITexture::pfColor4Byte:	dataType = Image::Int8;		format = Image::RGBA;			break;
	case ITexture::pfColor1Half:	dataType = Image::Float16;	format = Image::Luminance;		break;
	case ITexture::pfColor2Half:	dataType = Image::Float16;	format = Image::LuminanceAlpha;	break;
	case ITexture::pfColor3Half:	dataType = Image::Float16;	format = Image::RGB;			break;
	case ITexture::pfColor4Half:	dataType = Image::Float16;	format = Image::RGBA;			break;
	case ITexture::pfColor1Float:	dataType = Image::Float32;	format = Image::Luminance;		break;
	case ITexture::pfColor2Float:	dataType = Image::Float32;	format = Image::LuminanceAlpha;	break;
	case ITexture::pfColor3Float:	dataType = Image::Float32;	format = Image::RGB;			break;
	case ITexture::pfColor4Float:	dataType = Image::Float32;	format = Image::RGBA;			break;
	case ITexture::pfDepth16:		dataType = Image::Int16;	format = Image::Luminance;		break;
	case ITexture::pfDepth32:		dataType = Image::Int32;	format = Image::Luminance;		break;		
	}

	Image * image = new Image(w, h, d, dataType, format, mCompression, levels, isCubemap);

	for(uint32 i = 0 ; i < levels; ++i)
	{
		for(uint32 j = 0; j < faces; ++j)
		{
			const Image::Level& levelData = image->getLevel(i, j);
			subTarget = isCubemap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + j : mTarget; 
			if(compressed)
			{
				//check image size
				int compressedImgSize = 0;
				glGetTexLevelParameteriv(subTarget, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedImgSize);
				ASSERT(compressedImgSize == levelData.size);

				//obtain compressed image data
				glGetCompressedTexImage(subTarget, i, levelData.data);
			}
			else
			{
				//obtain image data
				glGetTexImage(subTarget, i, mFormat.format, mFormat.dataType, levelData.data);
			}
		}
	}

	return image;
}

} //namespace GL {

} //namespace Squirrel {
