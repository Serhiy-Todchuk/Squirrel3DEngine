#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQWORLD_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQWORLD_API
#else
#	if defined( SQWORLD_EXPORTS )
#    	define SQWORLD_API __declspec( dllexport )
#	else
#		define SQWORLD_API __declspec( dllimport )
#	endif
#endif
#else
#	if defined( SQWORLD_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQWORLD_API
#	else
#       define SQWORLD_API  __attribute__ ((visibility("default")))
#	endif
#endif
