//
// Interface of a class to access data (virtual file system)
//
#pragma once

#include	<Common/Data.h>
#include	<string>
#include	<map>
#include	<list>
#include	"macros.h"

#ifdef	_WIN32
	#pragma warning (disable:4996)
#endif

namespace Squirrel {

namespace FileSystem {

struct	FileInfo
{
	std::string absPath;
	std::string path;
	std::string name;
	std::string extension;
	bool isFolder;
	bool isHidden;
	bool isReadOnly;
};

class SQFILESYSTEM_API FileStorage
{
public:

	typedef std::map <std::string, FileStorage *> FILESTORAGES_MAP;
	static FILESTORAGES_MAP sFileStorages;

protected:
	FileStorage () {}
public:
	static FileStorage *	GetFileStorageForPath ( const std::string& path );

	virtual ~FileStorage () {}

	virtual	bool		isOk () const	{ return false;	}
	virtual	bool		hasFile  ( const std::string& name ) = 0;
	virtual	Data  *		getFile  ( const std::string& name ) = 0;
	virtual	bool		putFile  ( Data  * data, const std::string& name ) = 0;

	virtual const std::list<FileInfo>& getContent(const char_t* location) = 0;

	virtual	Data  *		getMappedFile  ( const std::string& name ) { return NULL; }
	virtual bool		supportsMappedFiles() = 0;
	virtual time_t		getFileModificationTime( const std::string& name ) = 0;

	// simple check whether the file exists
	static bool    IsFileExist ( const char * fileName );
	
	static time_t    GetFileModificationTime( const char * fileName );

protected:
	std::list<FileInfo> mContent;
};

}//namespace FileSystem { 

}//namespace Squirrel {