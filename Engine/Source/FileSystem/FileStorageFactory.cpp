
#include	"FileStorageFactory.h"
#include	"Folder.h"
#include	"ZipFileStorage.h"
#include	"Path.h"

namespace Squirrel {

namespace FileSystem { 

FileStorage * FileStorageFactory::GetFileStorageForPath(const std::string& path)
{
	FileStorage::FILESTORAGES_MAP::iterator it = FileStorage::sFileStorages.find( path );
	if(it == FileStorage::sFileStorages.end())
	{
		std::string ext = Path::GetExtension(path);
		if(ext == "zip")
		{
			ZipFileStorage * zipFile = new ZipFileStorage(path);
			FileStorage::sFileStorages[ path ] = zipFile;
			return zipFile;
		}
		else
		{
			Folder * newFolder = new Folder(path);
			FileStorage::sFileStorages[ path ] = newFolder;
			return newFolder;
		}
	}
	return it->second;
}

FileStorage * FileStorageFactory::GetFolderForPath(const std::string& path)
{
	Folder * newFolder = new Folder(path);
	return newFolder;
}

FileStorage * FileStorageFactory::GetZipFileStorageForPath(const std::string& path)
{
	ZipFileStorage * zipFile = new ZipFileStorage(path);
	return zipFile;
}

}//namespace FileSystem { 

}//namespace Squirrel {