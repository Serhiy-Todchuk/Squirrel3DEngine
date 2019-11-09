#pragma once

// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#if defined( SQFBXIMPORTER_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#	define SQFBXIMPORTER_API
#else
#	if defined( SQFBXIMPORTER_EXPORTS )
#    	define SQFBXIMPORTER_API __declspec( dllexport )
#	else
#		define SQFBXIMPORTER_API __declspec( dllimport )
#	endif
#endif