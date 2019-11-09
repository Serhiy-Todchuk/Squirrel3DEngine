#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQDAEIMPORTER_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQDAEIMPORTER_API
#else
#	if defined( SQDAEIMPORTER_EXPORTS )
#    	define SQDAEIMPORTER_API __declspec( dllexport )
#	else
#		define SQDAEIMPORTER_API __declspec( dllimport )
#	endif
#endif
#else
#	if defined( SQDAEIMPORTER_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQDAEIMPORTER_API
#	else
#       define SQDAEIMPORTER_API  __attribute__ ((visibility("default")))
#	endif
#endif
