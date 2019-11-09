//
//

#include	<stdio.h>

#ifdef	_WIN32
    #include	<fcntl.h>
    #include	<io.h>
#else
    #include	<unistd.h>
    #include	<sys/types.h>
    #include	<sys/stat.h>
    #include	<fcntl.h>
    #define	O_BINARY	0
#endif

#ifdef	__APPLE__
	#include	<stdlib.h>
#else
	#include	<malloc.h>
#endif

#include	<memory.h>
#include	<string.h>
#include	<algorithm>
#include	<list>
#include	<stack>
#include	"Path.h"
#include	<common/macros.h>
#include	<common/StringUtils.h>

#if __APPLE__
#include	<common/Mac/MacUtils.h>
#endif


namespace Squirrel {

namespace FileSystem { 

#ifdef	_WIN32
	char_t	pathSeparator = '\\';
	char_t	foreignPathSeparator = '/';
#else
	char_t	pathSeparator = '/';
	char_t	foreignPathSeparator = '\\';
#endif

std::string					Path ::sRoot;

Path :: Path ()
{
}

Path :: ~Path ()
{
}

std::string		Path::MakeUpPath(const std::string &path)
{
	if ( path.empty () )
		return path;

	std::string res( path );

	//make lowercase (TODO: remove?) 
	std::transform(res.begin(), res.end(), res.begin(), ::tolower);

	//split into path components
	std::list<std::string> pathComponents;
	int resStart = 0, resEnd = -1;
	for ( int i = 0; i < (int)res.length (); i++ )
	{
		bool isSeparator =	res [i] == foreignPathSeparator || 
							res [i] == pathSeparator;
		if(resEnd < 0)//if search for end
		{
			if(isSeparator)
			{
				resEnd = i - 1;
				std::string sub = res.substr(resStart, resEnd - resStart);
				pathComponents.push_back(sub);
				resStart = -1;//start searching next start
			}
		}
		if(resStart < 0)//if search for start
		{
			if(!isSeparator)
			{
				resStart = i;
				resEnd = -1;
			}
		}
	}

	//remove '..' folders
	std::stack<std::string> pathComponentsStack;
	std::list<std::string>::iterator it;
	for(it = pathComponents.begin(); it != pathComponents.end(); ++it)
	{
		if((*it) == "..")
		{
			pathComponentsStack.pop();
		}
		else
		{
			pathComponentsStack.push(*it);
		}
	}

	//join path components
	std::string resPath(pathComponentsStack.top());
	pathComponentsStack.pop();
	while(!pathComponentsStack.empty())
	{
		resPath = pathComponentsStack.top() + pathSeparator + resPath;
		pathComponentsStack.pop();
	}

	return resPath;
}

							// build filename from path and a name
std::string		Path::Combine ( const std::string& path, const std::string& name )
{
	if ( path.empty () )
		return name;
	if ( name.empty () )
		return path;

	int	pos = (int)path.length () - 1;

	std::string	res ( path );

	if ( path [pos] == '\\' || path [pos] == '/' )
		res += name;
	else
	{
		res += pathSeparator;
		res += name;
	}
	
	//make all separators in one style
	for ( int i = 0; i < (int)res.length (); i++ )
	{
		if ( res [i] == foreignPathSeparator )
		{
			res [i] = pathSeparator;
		}
	}

	return res;
}
								// extract folder from a filename
std::string	Path::GetDirectory ( const std::string& fullName )
{
	for ( int i = (int)fullName.length () - 2; i >= 0; i-- )
	{
		char	ch = fullName [i];

		if ( ch == '\\' || ch == '/' || ch == ':' )
			return fullName.substr ( 0, i + 1 );
	}

	return "";

}
							// extract filename and extension
std::string	Path::GetFileName ( const std::string& fullName )
{
	int	len = (int)fullName.length ();

	for ( int i = len - 1; i >= 0; i-- )
	{
		char	ch = fullName [i];

		if ( ch == '\\' || ch == '/' || ch == ':' )
			return fullName.substr ( i + 1 );
	}

	return fullName;
}

// get path without extension
std::string	Path::RemoveExtension ( const std::string& fullName )
{
	size_t	pos  = fullName.rfind('.');

	if ( pos == std::string :: npos )
		return fullName;

	return fullName.substr ( 0, pos );
}

// get only name without extension
std::string	Path::GetFileNameWithoutExtension ( const std::string& fullName )
{
	std::string	name = GetFileName ( fullName );

	return RemoveExtension( GetFileName ( fullName ) );
}

// get only extension
std::string Path::GetExtension(const std::string &path)
{
	size_t dotPos = path.rfind('.');
	if(dotPos != std::string::npos)
	{
		std::string extension = path.substr( dotPos + 1 );
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		return extension;
	}
	return std::string("");
}

std::string	Path :: GetAbsPath(const std::string &relativePath)
{
	return Combine( sRoot, relativePath );
}

void	Path :: InitRootPath ( const std::string& combineWith )
{
#ifdef _WIN32
	char strBuf[ 255 ];
	strBuf[0] = '\0';
	GetCurrentDirectory( 255, strBuf );
	sRoot = strBuf;
#elif __APPLE__
	sRoot = MacUtils::GetBundleParentFolder();
#endif

	if(combineWith.length() > 0)
	{
		sRoot = Combine( sRoot, combineWith );
	}
}

}//namespace FileSystem { 

}//namespace Squirrel {