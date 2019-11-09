#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQENGINE_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQENGINE_API
#else
#	if defined( SQENGINE_EXPORTS )
#    	define SQENGINE_API __declspec( dllexport )
#	else
#		define SQENGINE_API __declspec( dllimport )
#	endif
#endif
#else
#	if defined( SQENGINE_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQENGINE_API
#	else
#       define SQENGINE_API  __attribute__ ((visibility("default")))
#	endif
#endif
