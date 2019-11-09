//
// Interface of a class to access data (virtual file system)
//

#include	"FileStorage.h"
#include	"Path.h"
#include	"FileStorageFactory.h"
#include	<common/Log.h>
#include	<common/StringUtils.h>
#include	<sys/stat.h>

namespace Squirrel {
namespace FileSystem { 

FileStorage::FILESTORAGES_MAP FileStorage::sFileStorages;

FileStorage * FileStorage::GetFileStorageForPath ( const std::string& path )
{
	FILESTORAGES_MAP::iterator it = sFileStorages.find( path );
	if( it != sFileStorages.end() )
	{
		return it->second;
	}
	return NULL;
}

// simple check whether the file exists
bool    FileStorage::IsFileExist ( const char * fileName )
{
#ifdef  _WIN32
    struct _stat    buf;
    return _stat( fileName, &buf ) != -1;
#else
    struct stat buf;
    return stat ( fileName, &buf ) != -1;
#endif
}

time_t    FileStorage::GetFileModificationTime ( const char * fileName )
{
#ifdef  _WIN32
	struct _stat    buf;
	if( _stat( fileName, &buf ) == -1 )
		return 0;
#else
	struct stat buf;
	if( stat( fileName, &buf ) == -1 )
		return 0;
#endif
	return buf.st_mtime;
}

}//namespace FileSystem { 
}//namespace Squirrel {