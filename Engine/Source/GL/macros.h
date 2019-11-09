#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQOPENGL_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQOPENGL_API
#	define SQOPENGL_TEMPLATE
#else
#	if defined( SQOPENGL_EXPORTS )
#    	define SQOPENGL_API __declspec( dllexport )
#		define SQOPENGL_TEMPLATE
#	else
#		define SQOPENGL_API __declspec( dllimport )
#		define SQOPENGL_TEMPLATE extern
#	endif
#endif
#else
#	if defined( SQOPENGL_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQOPENGL_API
#       define SQOPENGL_TEMPLATE
#	else
#       define SQOPENGL_API  __attribute__ ((visibility("default")))
#       define SQOPENGL_TEMPLATE extern
#	endif
#endif