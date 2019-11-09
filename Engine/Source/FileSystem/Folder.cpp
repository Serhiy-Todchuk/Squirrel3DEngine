//
// Interface of a class to access data in folder
//

#include	"Folder.h"
#include	"Path.h"
#include	<common/Log.h>
#include	<common/StringUtils.h>
#include	<Common/Macros.h>
#include	<sys/stat.h>

#ifdef __APPLE__
# include	<CoreFoundation/CoreFoundation.h>
#endif

namespace Squirrel {
namespace FileSystem { 

Folder :: Folder ( const std::string& theFolder ) : FileStorage (), mFolder ( theFolder ) 
{

}

Folder :: ~Folder () 
{

}

Data  * Folder :: getFile  ( const std::string& name )
{
	std::string	newName = Path::Combine ( mFolder, name );
	if( !IsFileExist ( newName.c_str() ) ) return NULL;
	Data * fileData = new Data ( newName.c_str () );
	if(!fileData->isOk())
	{
		DELETE_PTR(fileData);
		return NULL;
	}
	return fileData;
}

Data  *		Folder :: getMappedFile  ( const std::string& name )
{
	if(name.length() == 0)
		return NULL;
	
	std::string	newName = Path::Combine ( mFolder, name );
	if( !IsFileExist ( newName.c_str() ) ) return NULL;
	Data * fileData = new Data ( newName.c_str (), true, true );
	if(!fileData->isOk())
	{
		DELETE_PTR(fileData);
		return NULL;
	}
	return fileData;
}

time_t		Folder :: getFileModificationTime( const std::string& name )
{
	if(name.length() == 0)
		return NULL;

	std::string	newName = Path::Combine ( mFolder, name );

	return GetFileModificationTime( newName.c_str() );
}

bool Folder :: putFile  ( Data  * data, const std::string& name )
{
	if(!data) return false;

	std::string	fullName = Path::Combine ( mFolder, name );
	
	return data->writeToFile(fullName.c_str()) >= 0;
}

bool Folder :: hasFile  ( const std::string& name )
{
	std::string	newName = Path::Combine ( mFolder, name );
	return IsFileExist ( newName.c_str() );
}

bool Folder :: isOk ()
{
	return true;//TODO: implement
}

void Folder :: refreshContent(const char_t* location)
{
	mContent.clear();

	std::string path = mFolder;
	if(location != NULL)
	{
		path = Path::Combine(mFolder, location);
	}

#ifdef  _WIN32
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char dirSpec[MAX_PATH];  // directory specification
	DWORD errorCode;

	strncpy (dirSpec, path.c_str(), strlen(path.c_str())+1);
	strncat (dirSpec, "\\*\0", 3);

	hFind = FindFirstFile(dirSpec, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		Log::Instance().error("Folder::getContent", "INVALID_HANDLE_VALUE");
	} 
	else 
	{
		//addFile(&FindFileData);
		while (FindNextFile(hFind, &findFileData) != 0) 
		{
			FileInfo fileInfo;
			fileInfo.name		= &findFileData.cFileName[0];
			fileInfo.extension	= Path::GetExtension( fileInfo.name );
			fileInfo.path		= Path::Combine( location, fileInfo.name );
			fileInfo.absPath	= Path::Combine( mFolder, fileInfo.path);
			fileInfo.isFolder	= (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			fileInfo.isHidden	= (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
			fileInfo.isReadOnly	= (findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
			mContent.push_back(fileInfo);
		}
	
		errorCode = GetLastError();
		FindClose(hFind);
		if (errorCode != ERROR_NO_MORE_FILES) 
		{
			Log::Instance().streamError("Folder::getContent") << "FindNextFile error" << errorCode;
			Log::Instance().flush();
		}
	}
#elif __APPLE__
	CFStringRef pathStr = CFStringCreateWithBytes(kCFAllocatorDefault,
												  (const UInt8 *)path.c_str(),
												  (CFIndex)path.length(),
												  kCFStringEncodingASCII,
												  false);
	
	CFURLRef pathURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
													 pathStr,
													 kCFURLPOSIXPathStyle,
													 true);
	
	CFURLEnumeratorRef enumerator = CFURLEnumeratorCreateForDirectoryURL(NULL,
																		 pathURL,
																		 kCFURLEnumeratorSkipInvisibles,
																		 NULL);
	
	const int strBuffSize = 256;
	char strBuff[strBuffSize];
	
	CFURLRef childURL = nil;
	CFURLEnumeratorResult enumeratorResult;
	
	do {
		enumeratorResult = CFURLEnumeratorGetNextURL(enumerator, &childURL, NULL);
		if (enumeratorResult == kCFURLEnumeratorSuccess) {
			
			CFStringRef childPath = CFURLCopyFileSystemPath(childURL, kCFURLPOSIXPathStyle);
			
			if(!CFStringGetCString(childPath, strBuff, (CFIndex)strBuffSize, kCFStringEncodingASCII))
				continue;
			
			std::string childPathStr = strBuff;
			
			FileInfo fileInfo;
			fileInfo.name		= Path::GetFileName(childPathStr);
			fileInfo.extension	= Path::GetExtension( fileInfo.name );
			fileInfo.path		= Path::Combine( location, fileInfo.name );
			fileInfo.absPath	= Path::Combine( mFolder, fileInfo.path);
			fileInfo.isFolder	= CFURLHasDirectoryPath(childURL);
			fileInfo.isHidden	= false;
			fileInfo.isReadOnly	= false;
			mContent.push_back(fileInfo);
			
			CFRelease(childPath);
			
		} else if (enumeratorResult == kCFURLEnumeratorError) {
			// A possible enhancement would be to present error-based items to the user.
		}
	} while (enumeratorResult != kCFURLEnumeratorEnd);

	CFRelease(enumerator);
	CFRelease(pathURL);
	CFRelease(pathStr);

#endif
}


const std::list<FileInfo>& Folder :: getContent(const char_t* location)
{
	refreshContent(location);
	return mContent;
}

}//namespace FileSystem {
}//namespace Squirrel {