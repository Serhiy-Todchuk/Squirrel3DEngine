//
// Basic class to handle reading of static data
//

#pragma once

#ifdef	_WIN32
	#pragma	warning (disable:4786 4996)
#endif

#include	"macros.h"
#include	<string>
#include	<vector>
#include	<Common/types.h>

namespace Squirrel {

namespace FileSystem { 

#define BastardsPathSeparator	'\\'
#define NormalPathSeparator		'/'

#ifdef	_WIN32
	#define PathSeparator  BastardsPathSeparator
#else
	#define PathSeparator  NormalPathSeparator
#endif

class SQFILESYSTEM_API Path
{
	static std::string sRoot;

	Path ();
	virtual ~Path ();
public:

	static void	InitRootPath ( const std::string& combineWith = "" );

	//Path utils
	static std::string	MakeUpPath(const std::string &path);
							// return absolute path from relative one
	static std::string	GetAbsPath(const std::string &path);
							// extract extension
	static std::string	GetExtension(const std::string &path);
							// build filename from path and a name
	static std::string	Combine ( const std::string& path, const std::string& name );
							// extract path from a filename
	static std::string	GetDirectory ( const std::string& fullName );
							// extract filename and extension
	static std::string	GetFileName ( const std::string& fullName );
							// get only name without extension
	static std::string	GetFileNameWithoutExtension ( const std::string& fullName );
							// get path without extension
	static std::string	RemoveExtension ( const std::string& fullName );
};

}//namespace FileSystem { 

}//namespace Squirrel {