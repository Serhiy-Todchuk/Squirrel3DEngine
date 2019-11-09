#pragma once

#if defined( SQCOMMON )
#	include <Common/macros.h>
#	define SQFILESYSTEM_API SQCOMMON_API
#else
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#ifdef _WIN32
#	if defined( SQFILESYSTEM_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#   	define SQFILESYSTEM_API
#   else
#   	if defined( SQFILESYSTEM_EXPORTS )
#       	define SQFILESYSTEM_API __declspec( dllexport )
#   	else
#			define SQFILESYSTEM_API __declspec( dllimport )
#   	endif
#	endif
#else
#	if defined( SQFILESYSTEM_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQFILESYSTEM_API
#	else
#       define SQFILESYSTEM_API  __attribute__ ((visibility("default")))
#	endif
#endif
#endif
