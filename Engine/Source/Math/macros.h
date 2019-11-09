#pragma once

#if defined( SQCOMMON )
#	include "../Common/macros.h"
#	define SQMATH_API SQCOMMON_API
#else
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#ifdef _WIN32
#	if defined( SQMATH_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#   	define SQMATH_API
#   else
#   	if defined( SQMATH_EXPORTS )
#       	define SQMATH_API __declspec( dllexport )
#   	else
#			define SQMATH_API __declspec( dllimport )
#   	endif
#	endif
#else
#	if defined( SQMATH_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQMATH_API
#	else
#       define SQMATH_API  __attribute__ ((visibility("default")))
#	endif
#endif
#endif