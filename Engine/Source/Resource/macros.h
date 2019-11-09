#pragma once

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQRESOURCE_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQRESOURCE_API
#	define SQRESOURCE_TEMPLATE
#else
#	if defined( SQRESOURCE_EXPORTS )
#    	define SQRESOURCE_API __declspec( dllexport )
#		define SQRESOURCE_TEMPLATE
#	else
#		define SQRESOURCE_API __declspec( dllimport )
#		define SQRESOURCE_TEMPLATE extern
#	endif
#endif
#else
#	if defined( SQRESOURCE_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQRESOURCE_API
#       define SQRESOURCE_TEMPLATE
#	else
#       define SQRESOURCE_API  __attribute__ ((visibility("default")))
#       define SQRESOURCE_TEMPLATE extern
#	endif
#endif
