//
// Simple class to access data in a zip file
//

#pragma once

#include	"FileStorage.h"
#include	<string>
#include	<list>

using namespace std;

namespace Squirrel {

namespace FileSystem { 

#pragma pack (push, 1)					// save current pack, set 1-byte packing

struct	ZipLocalFileHeader
{
	unsigned long	signature;
	unsigned short	versionNeededToExtract;
	unsigned short	generalPurposeBitFlag;
	unsigned short	compressionMethod;
	unsigned short	lastModFileTime;
	unsigned short	lastModFileDate;
	unsigned long	crc32;
	long			compressedSize;
	long			uncompressedSize;
	unsigned short	filenameLength;
	unsigned short	extraFieldLength;
};

struct	ZipDataDescriptor
{
	unsigned long	crc32;
	unsigned long	compressedSize;
	unsigned long	uncompressedSize;
};

struct	ZipCentralHeader				// of central header
{
	unsigned long	signature;
	unsigned short	versionMadeBy;
	unsigned short	versionNeededToExtract;
	unsigned short	generalPurposeBitFlag;
	unsigned short	compressionMethod;
	unsigned short	lastModFileTime;
	unsigned short	lastModFileDate;
	unsigned long	crc32;
	long			compressedSize;
	long			uncompressedSize;
	unsigned short	filenameLength;
	unsigned short	extraFieldLength;
	unsigned short  commentLength;
	unsigned short	diskNumberStart;
	unsigned short	internalFileAttibutes;
	unsigned long	externalFileAttributes;
	long			relativeLocalHeaderOffset;
};

struct	ZipEndOfCentralDir
{
	unsigned long	signature;
	unsigned short	diskNo;
	unsigned short	centralDirDiskNo;
	unsigned short	numEntriesOnDisk;
	unsigned short	numEntries;
	unsigned long	centralDirSize;
	long			centralDirOffset;
	unsigned short	commentLength;
};

#pragma	pack (pop)

class SQFILESYSTEM_API ZipFileStorage : public FileStorage
{
private:
	typedef	list <pair<string,ZipCentralHeader> >	ZipDir;

	string	fileName;								// name of zip file
	ZipDir	dir;									// contains directory of zip file
	bool	broken;

public:
	ZipFileStorage ( const string& zipName );
	virtual ~ZipFileStorage ( );

	virtual	bool	isOk () const	{ return !broken; }
	virtual	bool	hasFile  ( const std::string& name );
	virtual	Data  * getFile  ( const string& name );
	virtual	bool	putFile  ( Data  * data, const std::string& name ) { return false; }

	virtual const std::list<FileInfo>& getContent(const char_t* location);

	virtual time_t		getFileModificationTime( const std::string& name ) { return 0; }//TODO: implement

	virtual bool		supportsMappedFiles() { return false; }

private:
	void	readDirectory ( int file );
	Data  * readEntry     ( const ZipCentralHeader& hdr );
};

}//namespace FileSystem { 

}//namespace Squirrel {