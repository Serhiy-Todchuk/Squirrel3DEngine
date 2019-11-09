
#pragma once

#ifdef	_WIN32
	#pragma	warning (disable: 4996)
#endif

#include	<string>
#include	"FileStorage.h"

namespace Squirrel {

namespace FileSystem { 

class SQFILESYSTEM_API FileStorageFactory
{
public:
	static FileStorage * GetFileStorageForPath(const std::string& path);
	static FileStorage * GetFolderForPath(const std::string& path);
	static FileStorage * GetZipFileStorageForPath(const std::string& path);
};

}//namespace FileSystem { 

}//namespace Squirrel {