#pragma once

#if defined( SQCOMMON )
#	include <Common/macros.h>
#	define SQRENDER_API SQCOMMON_API
#	define SQRENDER_TEMPLATE SQCOMMON_TEMPLATE
#else
#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#	if defined( SQRENDER_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#   	define SQRENDER_API
#   	define SQRENDER_TEMPLATE
#   else
#   	if defined( SQRENDER_EXPORTS )
#       	define SQRENDER_API __declspec( dllexport )
#   		define SQRENDER_TEMPLATE
#   	else
#			define SQRENDER_API __declspec( dllimport )
#   		define SQRENDER_TEMPLATE extern
#   	endif
#	endif
#else
#	if defined( SQRENDER_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQRENDER_API
#       define SQRENDER_TEMPLATE
#	else
#       define SQRENDER_API  __attribute__ ((visibility("default")))
#       define SQRENDER_TEMPLATE extern
#	endif
#endif
#endif