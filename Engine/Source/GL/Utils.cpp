// Utils.cpp: implementation of the Utils class.
//
//////////////////////////////////////////////////////////////////////
#include "Utils.h"
#include <Math/BasicUtils.h>
#include <Common/Log.h>
#include <Common/StringUtils.h>

namespace Squirrel {

namespace GL {


struct token_string
{
   GLuint Token;
   const char* String;
};

static const struct token_string Errors[]=
{
   /* GL */
   {GL_NO_ERROR, "no error"},
   {GL_INVALID_ENUM, "invalid enumerant"},
   {GL_INVALID_VALUE, "invalid value"},
   {GL_INVALID_OPERATION, "invalid operation"},
//   {GL_STACK_OVERFLOW, "stack overflow"},
//   {GL_STACK_UNDERFLOW, "stack underflow"},
   {GL_OUT_OF_MEMORY, "out of memory"},
   {0x506, "invalid framebuffer operation"},
   { static_cast<GLuint>(0), NULL } /* end of list indicator */
};

const char* Utils::ErrorString(GLenum errorCode)
{
   int i;

   for (i=0; Errors[i].String; i++)
   {
      if (Errors[i].Token==errorCode)
      {
         return Errors[i].String;
      }
   }

   return NULL;
}

bool Utils::CheckGLError(const char_t * msg)
{
	GLenum errCode = glGetError();
	if(errCode != GL_NO_ERROR)
	{
		const char_t * str = Utils::ErrorString(errCode);
		Log::Instance().streamError("CheckGLError") << msg << " " << str;
		Log::Instance().flush();
		return true;
	}
	return false;
}

int Utils::ConvertVersion(tuple2i version)
{
	int number = version.x * 100;
	number += (version.y < 10) ? (version.y * 10) : version.y;
	return number;
}

int Utils::GetGLSLVersion()
{
	int glVersion = ConvertVersion(GetGLVersion());
	switch (glVersion) {
		case 200: return 110;
		case 210: return 120;
		case 300: return 130;
		case 310: return 140;
		case 320: return 150;
		default:
			if(glVersion >= 330)
				return glVersion;
			else
				return 0;
			break;
	}
}
	
tuple2i Utils::GetGLVersion()
{
#ifdef SUPPORT_GL3
	int majorVersion = 0;
	int minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	CHECK_GL_ERROR;
	return tuple2i(majorVersion, minorVersion);
#else
	const GLubyte * versionString = glGetString(GL_VERSION);
	CHECK_GL_ERROR;
	int majorVersion = (int)versionString[0] - (int)'0';
	int minorVersion = (int)versionString[2] - (int)'0';
	return tuple2i(majorVersion, minorVersion);
#endif
}
	
bool Utils::IsExtensionSupported(const char_t * extensionName)
{	
	size_t len = strlen(extensionName);
	char_t * str = NULL;
	int num = 0;
	size_t size = 0;
	
	glGetIntegerv(GL_NUM_EXTENSIONS, &num);
	if (glGetError() == GL_NO_ERROR && num > 0)
	{
		for (int i = 0; i < num; i++)
		{
			str = (char_t *)glGetStringi(GL_EXTENSIONS, i);
			size = strlen(str);
			if (len == size && strcmp(str, extensionName) == 0)
			{
				CHECK_GL_ERROR;
				return true;
			}
		}
		CHECK_GL_ERROR;
		return false;
	}
	
	str = (char_t *)glGetString(GL_EXTENSIONS);
	CHECK_GL_ERROR;
	if (0 == str)
		return false;
	char_t * end = str + strlen(str);
	while (str < end)
	{
		size = strFindChar(str, ' ');
		if (len == size && strcmp(str, extensionName))
			return true;
		str += size + 1;
	}
	return false;
}

GLenum	Utils::fpAlphaFormatWithPrecision ( int bits )
{
#ifdef SUPPORT_GL3
	return (bits == 16 ? GL_R16F : GL_R32F);
#else
#ifdef	MACOSX
	if ( isExtensionSupported ( "GL_APPLE_float_pixels" ) )
		return (bits == 16 ? GL_ALPHA_FLOAT16_APPLE : GL_ALPHA_FLOAT32_APPLE);
#else
	if ( GLEE_ARB_texture_float )
		return (bits == 16 ? GL_ALPHA16F_ARB : GL_ALPHA32F_ARB);
	if ( GLEE_NV_float_buffer )
		return (bits == 16 ? GL_FLOAT_R16_NV : GL_FLOAT_R32_NV );
	if ( GLEE_ATI_texture_float )
		return (bits == 16 ? GL_ALPHA_FLOAT16_ATI : GL_ALPHA_FLOAT32_ATI);
#endif
#endif
	return 0;
}

GLenum	Utils::fpRGFormatWithPrecision ( int bits )
{
#ifdef SUPPORT_GL3
	return (bits == 16 ? GL_RG16F : GL_RG32F);
#else
#ifdef	MACOSX
	if ( isExtensionSupported ( "GL_APPLE_float_pixels" ) )
		return (bits == 16 ? GL_RG_FLOAT16_APPLE : GL_RG_FLOAT32_APPLE);
#else
	if ( GLEE_ARB_texture_float )
		return (bits == 16 ? GL_LUMINANCE_ALPHA16F_ARB : GL_LUMINANCE_ALPHA32F_ARB);
	if ( GLEE_NV_float_buffer )
		return (bits == 16 ? GL_FLOAT_RG16_NV : GL_FLOAT_RG32_NV );
	if ( GLEE_ATI_texture_float )
		return (bits == 16 ? GL_LUMINANCE_ALPHA_FLOAT16_ATI : GL_LUMINANCE_ALPHA_FLOAT16_ATI);
#endif
#endif
	return 0;
}

GLenum	Utils::fpRGBFormatWithPrecision ( int bits )
{
#ifdef SUPPORT_GL3
	return (bits == 16 ? GL_RGB16F : GL_RGB32F);
#else
#ifdef	MACOSX
	if ( isExtensionSupported ( "GL_APPLE_float_pixels" ) )
		return (bits == 16 ? GL_RGB_FLOAT16_APPLE : GL_RGB_FLOAT32_APPLE);
#else
	if ( GLEE_ARB_texture_float )
		return (bits == 16 ? GL_RGB16F_ARB : GL_RGB32F_ARB);
	if ( GLEE_NV_float_buffer )
		return (bits == 16 ? GL_FLOAT_RGB16_NV : GL_FLOAT_RGB32_NV );
	if ( GLEE_ATI_texture_float )
		return (bits == 16 ? GL_RGB_FLOAT16_ATI : GL_RGB_FLOAT32_ATI );
#endif
#endif
	return 0;
}

GLenum	Utils::fpRGBAFormatWithPrecision ( int bits )
{
#ifdef SUPPORT_GL3
	return (bits == 16 ? GL_RGBA16F : GL_RGBA32F);
#else
#ifdef	MACOSX
	if ( isExtensionSupported ( "GL_APPLE_float_pixels" ) )
		return (bits == 16 ? GL_RGBA_FLOAT16_APPLE : GL_RGBA_FLOAT32_APPLE);
#else
	if ( GLEE_ARB_texture_float )
		return (bits == 16 ? GL_RGBA16F_ARB : GL_RGBA32F_ARB);
	if ( GLEE_NV_float_buffer )
		return (bits == 16 ? GL_FLOAT_RGBA16_NV : GL_FLOAT_RGBA32_NV );
	if ( GLEE_ATI_texture_float )
		return (bits == 16 ? GL_RGBA_FLOAT16_ATI : GL_RGBA_FLOAT32_ATI);
#endif
#endif
	return 0;
}

bool Utils::isFloatTextureSupported()
{
#ifdef SUPPORT_GL3
	return true;
#else
#ifdef	MACOSX
	if ( isExtensionSupported ( "GL_APPLE_float_pixels" ) )	return true;
#else
	if ( GLEE_ARB_texture_float )	return true;
	if ( GLEE_NV_float_buffer )		return true;
	if ( GLEE_ATI_texture_float )	return true;
#endif
#endif
	return false;
}

GLenum	Utils::getTexRectTarget ( )
{
#ifdef SUPPORT_GL3
	if(GetGLVersion().x == 3 && GetGLVersion().y == 0)
	{
		//TODO
	}
	return GL_TEXTURE_RECTANGLE;
#else
	if ( GLEE_ARB_texture_rectangle )
		return GL_TEXTURE_RECTANGLE_ARB;
	else if ( GLEE_NV_texture_rectangle )
		return GL_TEXTURE_RECTANGLE_NV;
#endif
	return 0;
}

} //namespace GL {

} //namespace Squirrel {
