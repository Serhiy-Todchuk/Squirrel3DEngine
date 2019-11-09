#pragma once

#ifdef	_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN   1          // Exclude rarely-used stuff from Windows headers
# endif
# include <windows.h>
#endif

#include <string>
#include <stdexcept>

typedef signed	 int	_ID;

typedef unsigned int     uint;
typedef unsigned long    ulong;
typedef unsigned short   ushort;

typedef	unsigned char		uint8;
typedef	signed	 char		int8;
typedef	unsigned short		uint16;
typedef	signed	 short		int16;
typedef	unsigned int		uint32;
typedef	signed	 int		int32;

#ifdef	_WIN32
 typedef	unsigned __int64	uint64;
 typedef	signed	 __int64	int64;
#else
 typedef	unsigned long long	uint64;
 typedef	signed   long long	int64;
#endif

typedef	unsigned char		byte;
typedef	uint16				word;
typedef	uint32				dword;

typedef	uint16				float16;
typedef	float				float32;
typedef	double				float64;
typedef	long double			float80;

typedef	std::string::value_type		char_t;