#pragma once

#ifdef	_WIN32
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN   1          // Exclude rarely-used stuff from Windows headers
#	endif
#	include <windows.h>
#	pragma warning( disable: 4251 )
#else
#   include <assert.h>
#endif

#include <cstddef>

#define TYPE_CAST			static_cast

#define STRINGIFY(x)		#x
#define TOSTRING(x)			STRINGIFY(x)

#define TOKENPASTE(x, y)	x ## y
#define TOKENPASTE2(x, y)	TOKENPASTE(x, y)

#ifdef _DEBUG
#   ifdef _WIN32
#		define ASSERT(exp)		if(!(exp)) _ASSERT(false);
#   else
#		define ASSERT(exp)		if(!(exp)) assert(false);
#   endif
#else
#   ifdef _WIN32
#       define ASSERT(exp)		if(!(exp)) MessageBox(NULL, ("Assert at line " TOSTRING(__LINE__) ", of file " __FILE__), "Error", MB_OK);
#   else
#       define ASSERT(exp)      assert(exp);
//#		define ASSERT(exp)		if(!(exp)) { throw new std::exception("Assert failed at line " TOSTRING(__LINE__) ", of file " __FILE__ "!"); }
#   endif
#endif

#define DELETE_PTR(ptr)	{	if((ptr)!=NULL) { delete  (ptr); (ptr)=NULL; }	}
#define DELETE_ARR(arr)	{	if((arr)!=NULL) { delete[](arr); (arr)=NULL; }	}

#define FOREACH(iteratorType, iterator, collection) for(iteratorType iterator = collection.begin(); iterator != collection.end(); ++iterator)

#ifdef _WIN32
// If we're not including this from a client build, specify that the stuff
// should get exported. Otherwise, import it.
#	if defined( SQCOMMON_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#   	define SQCOMMON_API
#   	define SQCOMMON_TEMPLATE
#   else
#   	if defined( SQCOMMON_EXPORTS )
#       	define SQCOMMON_API __declspec( dllexport )
#   		define SQCOMMON_TEMPLATE
#   	else
#			define SQCOMMON_API __declspec( dllimport )
#   		define SQCOMMON_TEMPLATE extern
#   	endif
#	endif
#else
#	if defined( SQCOMMON_STATIC_LIB ) || defined( SQ_STATIC_IMPORT )
#       define SQCOMMON_API
#       define SQCOMMON_TEMPLATE
#	else
#       define SQCOMMON_API  __attribute__ ((visibility("default")))
#       define SQCOMMON_TEMPLATE extern
#	endif
#endif

#if defined(nullptr_t) || (__cplusplus > 199711L) || defined(HACK_GCC_ITS_CPP0X) || defined(_HAS_CPP0X)
#define SQ_CPP0X 1
#endif