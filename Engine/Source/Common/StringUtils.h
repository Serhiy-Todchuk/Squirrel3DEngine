
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <string>
#include <sstream>

namespace Squirrel {

class MakeString {
public:
	template<class T>
	MakeString& operator<< (const T& arg) {
		mStream << arg;
		return *this;
	}
	operator std::string() const {
		return mStream.str();
	}
protected:
	std::stringstream mStream;
};
	
#define StrDefaultLen					255

#define StrLen(str)					strlen(str)
#define StrCpy(dst,src)				strcpy_s(dst,StrLen(src)+1, src)
#define StrCat(dst,src)				strcat_s(dst,StrLen(src)+1, src)
#define StrCmp(dst,src)				strcmp(dst,src)
#define StrPrint(dst,format,params)	sprintf_s(dst,StrDefaultLen,format,params)
#define StrScan(dst,format,params)	sscanf_s(dst,StrDefaultLen,format,params)
	
inline int strFindChar(const char_t * str, char_t ch)
{
	int size = 0;
    const char_t * ptr = str;
    while ( *ptr != '\0' )
	{
		if(*ptr == ch)
			break;
		++size;
		++ptr;
	}
    return size;
}
	
inline void trim(std::string& str)
{
	const std::string::value_type * spaces = " \t";
	std::string::size_type pos = str.find_last_not_of(spaces);
	if(pos != std::string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(spaces);
		if(pos != std::string::npos) str.erase(0, pos);
	}
	else str.erase(str.begin(), str.end());
}

template <class _TStringsContainer>
inline void split(const std::string& source, const char_t * separators, _TStringsContainer& output)
{
	if(source.length() == 0)
		return;

	std::string::size_type pos = source.find_first_of(separators);
	std::string::size_type end = std::string::npos;
	
	if(pos == std::string::npos)
	{
		output.push_back(source);
		return;
	}
	
	pos = source.find_first_not_of(separators);
	
	while(pos != std::string::npos)
	{
		end = source.find_first_of(separators, pos);
		
		std::string node = source.substr(pos, end - pos);

		if(node.length() > 0)
			output.push_back(node);
		
		pos = source.find_first_not_of(separators, end);
	}
	
	if(end < source.size() - 1)
	{
		output.push_back( source.substr(end + 1) );
	}
}
	
inline char * StrCreate(const size_t len)
{
	char * buf = new char[len];
	buf[len-1]='\0';
	return buf;
}
inline void StrDelete(char * buf)
{
	if(buf) delete[]buf;
}
inline char * StrResize(char * oldbuf, const size_t len)
{
	if(len<1) return 0;
	StrDelete(oldbuf);
	char * buf = StrCreate(len);
	buf[len-1]='\0';
	return buf;
}
inline char * StrLwr ( char * str )
{
    char * ptr = str;
    while ( *ptr != '\0' )
		*ptr++ = tolower ( *ptr );
    return str;
}

/*
	int strlen(const char * s)
	{
		int i=0;
		while(s[i]!='\0')
			i++;
		return i;
	}

	void strcpy(char * dst, const char * src)
	{
		int i=0;
		while(src[i]!='\0')
		{
			dst[i]=src[i];
			i++;
		}
		dst[i]='\0';
	}

	void strcat(char * dst, const char * src)
	{
		int i=s_len(dst);
		int j=0;
		while(src[j]!='\0')
		{
			dst[i]=src[j];
			j++;
			i++;
		}
		dst[i]='\0';
	}

	int strcmp(const char * dst, const char * src)
	{
		int counter=0;
		int i=0;
		while((src[i]!='\0') && (dst[i]!='\0'))
		{
			if(src[i]!=dst[i])
				counter++;
			i++;
		}
		return counter;
	}
//*/

} //namespace Squirrel {