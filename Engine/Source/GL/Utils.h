#pragma once

#include "OpenGL.h"
#include <common/tuple.h>

namespace Squirrel {

namespace GL {

#ifdef _DEBUG
#	define CHECK_GL_ERROR		GL::Utils::CheckGLError( ("GLError at " TOSTRING(__LINE__) ", file " __FILE__) )
#else
#	define CHECK_GL_ERROR
#endif

class SQOPENGL_API Utils  
{
public:

	static int ConvertVersion(tuple2i version);
	static tuple2i GetGLVersion();
	static int GetGLSLVersion();
	static bool IsExtensionSupported(const char_t * extensionName);
	
	static const char* ErrorString(GLenum errorCode);
	static bool CheckGLError(const char_t * msg);

	static GLenum fpAlphaFormatWithPrecision ( int bits = 32 );
	static GLenum fpRGFormatWithPrecision  ( int bits = 32 );
	static GLenum fpRGBFormatWithPrecision  ( int bits = 32 );
	static GLenum fpRGBAFormatWithPrecision  ( int bits = 32 );

	static bool isFloatTextureSupported();

	static GLenum getTexRectTarget();

	//static int isExtensionSupported(const char *extension);
};

} //namespace GL {

} //namespace Squirrel {
