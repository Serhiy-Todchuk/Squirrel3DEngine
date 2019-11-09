//
// Basic class to handle reading of static data
//

#include	<stdio.h>

#ifdef	_WIN32
    #include	<fcntl.h>
    #include	<io.h>
    #include	<sys/stat.h>
#else
    #include	<unistd.h>
    #include	<sys/types.h>
    #include	<sys/stat.h>
    #include	<fcntl.h>
	#include	<sys/mman.h>
    #define	O_BINARY	0
#endif

#ifdef	__APPLE__
	#include	<stdlib.h>
#else
	#include	<malloc.h>
#endif

#include	<memory.h>
#include	<string.h>
#include	"Data.h"

#ifdef	_WIN32
	#pragma	warning (disable:4996)
#endif

namespace Squirrel {

Data :: Data ( void * ptr, size_t len )
{
	initMembers();

	mBits   = (byte *) ptr;
	mLength = len;
	mCapacity = len;
	if(ptr == NULL)
		realloc(len);
}

Data :: ~Data ()
{
	if(isMappedFile())
	{
#ifdef	_WIN32
		UnmapViewOfFile(mBits);
		CloseHandle(mFileMapping);
		CloseHandle(mMappedFile);
#else
		munmap(mBits, mLength);
		close(mMappedFile);
#endif
	}
	else
	{
		if(mBits)   
			delete[]mBits;
	}
}

Data :: Data ( const char_t * fileName )
{
	initMembers();
	readFile(fileName);
}

Data :: Data ( const char_t * fileName, bool mapFile, bool readOnly )
{
	initMembers();

	if(mapFile)
	{
#ifdef	_WIN32
		mMappedFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ,  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(!mMappedFile) 
			return;

		mFileMapping = CreateFileMapping(mMappedFile, NULL, PAGE_READONLY, 0,0, NULL);
		if(!mFileMapping) { 
			CloseHandle(mMappedFile); 
			mMappedFile = 0;
			return;
		}

		mBits = (byte *)MapViewOfFile(mFileMapping, FILE_MAP_READ, 0,0,0);
		if(!mBits) { 
			CloseHandle(mMappedFile); 
			CloseHandle(mFileMapping); 
			mMappedFile = 0;
			return;
		}

		mLength = GetFileSize(mMappedFile, NULL);
		mCapacity = mLength;
#else
		mMappedFile = open(fileName, O_RDONLY);
	
		if(mMappedFile < 0)
			return;
	
		fcntl(mMappedFile, F_NOCACHE, 1); // не кешируем мы наше чтение текстур, потому что читаем линейно один раз то
		fcntl(mMappedFile, F_RDAHEAD, 1); // заставляем читать на перёд

		struct stat statbuf;
	
		if(fstat(mMappedFile, &statbuf) < 0)
			return;
	
		mBits = (byte *)mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, mMappedFile, 0);
	
		mLength = statbuf.st_size;
		mCapacity = mLength;
#endif
	}
	else
	{
		readFile(fileName);
	}
}

void Data :: initMembers()
{
	mBits				= 0;
	mLength				= 0;
	mPos				= 0;
	mCapacityIncrement	= 0;
	mCapacity			= 0;
	mMappedFile			= 0;
}

size_t Data::writeToFile ( const char_t * fileName )
{
	if( mBits == NULL || mLength == 0 ) return -1;
												// make a fix for windows to replace '/' in mFileName path
												// to windoze style '\\' if under windoze
	char_t * name = strdup ( fileName );

#ifdef	_WIN32
	char_t * ptr;

	while ( ( ptr = strchr ( name, '/' ) ) != NULL )
		*ptr = '\\';

	int	fd = open ( name, _O_WRONLY | _O_CREAT | O_BINARY | O_TRUNC, _S_IREAD | _S_IWRITE );
#else

	int	fd = open ( name, O_WRONLY | O_CREAT | O_BINARY | O_TRUNC, S_IREAD | S_IWRITE );
#endif
	//int error = errno;

	mFileName = name;

	free ( name );

	if ( fd == -1 )
		return -2;

	::write( fd, mBits, mLength );

	close ( fd );

	return mLength;
}

int  Data::readFile ( const char_t * fileName )
{
	char_t * name = strdup ( fileName );
	// make a fix for windows to replace '/' in mFileName path
	// to windoze style '\\' if under windoze

#ifdef	_WIN32
	char_t * ptr;

	while ( ( ptr = strchr ( name, '/' ) ) != NULL )
		*ptr = '\\';
#endif

	mBits   = NULL;
	mLength = 0;
	mPos    = 0;
	mFileName   = name;

	int	fd = open ( name, O_RDONLY | O_BINARY );

	free ( name );

	if ( fd == -1 )
		return -1;

#ifndef _WIN32
	struct	stat statBuf;
	
	fstat ( fd, &statBuf );
	
	long	len = statBuf.st_size; 
#else	
	long	len = filelength ( fd );
#endif

	if ( len < 1 )
	{
		close ( fd );

		return 0;
	}

	mBits = (byte *) malloc ( len );

	if ( mBits == NULL )
	{
		close ( fd );

		return 0;
	}

	mCapacity = len;
	mLength = read ( fd, mBits, len );

	close ( fd );

	return 1;
}


bool	Data :: isOk () const
{
	return mBits != NULL;
}

void * Data :: getPtr ( int offs ) const
{
	if ( offs < 0 || offs >= mLength )
		return NULL;

	return mBits + offs;
}

size_t	Data :: readBytes ( void * ptr, size_t len )
{
	if ( mPos >= mLength )
		return 0;

	if ( mPos + len > mLength )
		len = mLength - mPos;

	memcpy ( ptr, mBits + mPos, len );

	mPos += len;

	return len;
}

bool	Data ::  readString ( std::string& str, char_t term )
{
	if ( mPos >= mLength )
		return false;

	str = "";

	while ( mPos < mLength && mBits [mPos] != term )
		str += mBits [mPos++];

	if ( mPos < mLength && mBits [mPos] == term )
		mPos ++;
													// skin OA part of line terminator (0D,0A)
	if ( term == '\r' && mPos + 1 < mLength && mBits [mPos+1] == '\n' )
		mPos++;

	return true;
}

byte	Data ::  readByte ()
{
	if ( mPos < mLength )
		return mBits [mPos++];
	else
		return -1;
}

void * Data ::  getPtr () const
{
	return mBits + mPos;
}

int 	Data ::  seekUntil(char_t term)
{
	if ( mPos >= mLength )
		return 0;

	while ( mPos < mLength && mBits [mPos] != term )
		++mPos;

	if ( mPos < mLength && mBits [mPos] == term )
		++mPos;
													// skin OA part of line terminator (0D,0A)
	if ( term == '\r' && mPos + 1 < mLength && mBits [mPos+1] == '\n' )
		++mPos;

	return mPos;
}

int 	Data ::  seekSkipAnyOf(const char_t * chars)
{
	if ( mPos >= mLength )
		return 0;

	if ( chars == NULL )
		return 0;

	size_t charsLen = strlen(chars);
	if ( charsLen == 0 )
		return 0;

	while ( mPos < mLength )
	{
		char_t srcChar = mBits [mPos];
		bool anyOf = false;
		for ( char_t i = 0; i < charsLen; ++i )
		{
			if(srcChar == chars[i] )
			{
				anyOf = true;
				break;
			}
		}
		if(!anyOf)
		{
			break;
		}
		++mPos;
	}

	return mPos;
}

int	Data ::  seekCur ( int delta )
{
	mPos += delta;

	if ( mPos > mLength )
		mPos = mLength;

	return mPos;
}

int	Data ::  seekAbs ( int offs )
{
	mPos = offs;

	if ( mPos > mLength )
		mPos = mLength;

	return mPos;
}

std::string Data ::  readString()
{
	std::string str;
	readString( str, '\0' );
	return str;
}

void Data ::  realloc(size_t newLen)
{
	if(newLen<1) return;
	byte * newBits = new byte[newLen];
	if(mBits)
	{
		memcpy( newBits, mBits, (newLen < mLength) ? newLen : mLength );
		delete[]mBits;
	}
	mBits		= newBits;
	mLength		= newLen;
	mCapacity	= newLen;
	if( mPos > newLen )
		mPos = 0;
}

void Data ::  queryCapacity(size_t bytesNum)
{
	size_t targetLen = mPos + bytesNum;
	if ( targetLen > mLength )
	{
		if( targetLen > mCapacity )
		{
			size_t bytesNeed = targetLen - mCapacity;
			size_t newCapacity = mCapacity + bytesNeed;
			if(bytesNeed < mCapacityIncrement)
			{
				newCapacity = mCapacity + mCapacityIncrement;
			}
			realloc( newCapacity );
		}
		mLength = targetLen;
	}
}

void Data ::  putData (const void * data, size_t dataLen)
{
	if ( data == NULL || dataLen < 1 ) return;
	queryCapacity( dataLen );
	memcpy( mBits + mPos, data, dataLen );
	mPos += dataLen;
}

void Data ::  putString (const std::string& str)
{
	putData(str.c_str(),(int)str.length());
	putByte('\0');
}

void	Data ::  putByte (byte b)
{
	queryCapacity( 1 );
	mBits [mPos++] = b;
}

}//namespace Squirrel {