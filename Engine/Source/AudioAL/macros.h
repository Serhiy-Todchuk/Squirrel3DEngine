#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQOPENAL_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQOPENAL_API
#	define SQOPENAL_TEMPLATE
#else
#	if defined( SQOPENAL_EXPORTS )
#    	define SQOPENAL_API __declspec( dllexport )
#		define SQOPENAL_TEMPLATE
#	else
#		define SQOPENAL_API __declspec( dllimport )
#		define SQOPENAL_TEMPLATE extern
#	endif
#endif
#else
#	if defined( SQOPENAL_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQOPENAL_API
#       define SQOPENAL_TEMPLATE
#	else
#       define SQOPENAL_API  __attribute__ ((visibility("default")))
#       define SQOPENAL_TEMPLATE extern
#	endif
#endif