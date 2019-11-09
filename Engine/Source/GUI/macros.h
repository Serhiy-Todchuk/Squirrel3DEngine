#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQGUI_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQGUI_API
#else
#	if defined( SQGUI_EXPORTS )
#    	define SQGUI_API __declspec( dllexport )
#	else
#		define SQGUI_API __declspec( dllimport )
#	endif
#endif
#else
#	if defined( SQGUI_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQGUI_API
#	else
#       define SQGUI_API  __attribute__ ((visibility("default")))
#	endif
#endif
