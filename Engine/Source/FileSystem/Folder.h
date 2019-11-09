//
// Interface of a class to access data (virtual file system)
//
#pragma once

#include	"FileStorage.h"

namespace Squirrel {

namespace FileSystem { 

class SQFILESYSTEM_API Folder : public FileStorage
{
	std::string	mFolder;

	void refreshContent(const char_t* location);

public:
	Folder ( const std::string& theFolder );
	virtual ~Folder ( );

	virtual	bool		isOk ();
	virtual	bool		hasFile  ( const std::string& name );
	virtual	Data  *		getFile  ( const std::string& name );
	virtual	bool		putFile  ( Data  * data, const std::string& name );

	virtual	Data  *		getMappedFile  ( const std::string& name );
	virtual bool		supportsMappedFiles() { return true; }

	virtual time_t		getFileModificationTime( const std::string& name );

	virtual const std::list<FileInfo>& getContent(const char_t* location);
};

}//namespace FileSystem { 

}//namespace Squirrel {