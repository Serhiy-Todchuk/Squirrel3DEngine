#pragma once

#if defined( SQCOMMON )
#	include "../Common/macros.h"
#	define SQREFLECTION_API SQCOMMON_API
#else

/*
#	ifndef TOSTRING
#		define STRINGIFY(x)		#x
#		define TOSTRING(x)			STRINGIFY(x)
#	endif

#	ifndef TOKENPASTE
#		define TOKENPASTE(x, y)	x ## y
#		define TOKENPASTE2(x, y)	TOKENPASTE(x, y)
#	endif

#	ifndef ASSERT
#		define ASSERT(exp)		if(!exp) { throw new Exception("Assert failed at line " TOSTRING(__LINE__) ", of file " __FILE__ "!"); }
#	endif
*/

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#	if defined( SQREFLECTION_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#   	define SQREFLECTION_API
#   else
#   	if defined( SQREFLECTION_EXPORTS )
#       	define SQREFLECTION_API __declspec( dllexport )
#   	else
#			define SQREFLECTION_API __declspec( dllimport )
#   	endif
#	endif
#else
#	if defined( SQREFLECTION_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQREFLECTION_API
#	else
#       define SQREFLECTION_API  __attribute__ ((visibility("default")))
#	endif
#endif
#endif