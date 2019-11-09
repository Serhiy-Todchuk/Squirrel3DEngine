#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQEDITOR_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQEDITOR_API
#else
#	if defined( SQEDITOR_EXPORTS )
#    	define SQEDITOR_API __declspec( dllexport )
#	else
#		define SQEDITOR_API __declspec( dllimport )
#	endif
#endif
#else
#	if defined( SQEDITOR_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQEDITOR_API
#	else
#       define SQEDITOR_API  __attribute__ ((visibility("default")))
#	endif
#endif
