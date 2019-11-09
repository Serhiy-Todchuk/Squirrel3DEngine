
#ifdef	_WIN32
    #include	<fcntl.h>
    #include	<io.h>
    #include	<sys/stat.h>
	#include	<algorithm>
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

#include	<stdio.h>
#include	<ctype.h>

#include	"ZipFileStorage.h"
#include	"Path.h"
#include	<zlib/zlib.h>

#define	LOCAL_ZIP_SIGNATURE		0x04034B50
#define	CENTRAL_ZIP_SIGNATURE	0x02014B50
#define	END_CENTRAL_SIGNATURE	0x06054B50
#define	EXTD_LOCAL_SIGNATURE	0x08074B50

#define DEF_WBITS	15					// Default LZ77 window size
#define ZIP_STORE	0					// 'STORED' method id
#define ZIP_DEFLATE	8					// 'DEFLATE' method id

#define	CHUNK_SIZE	1024				// chunk size for reading zip file from end

namespace Squirrel {

namespace FileSystem { 

ZipFileStorage :: ZipFileStorage ( const string& zipName ) : FileStorage(), fileName ( zipName )
{
	int	file = open ( fileName.c_str (), O_BINARY | O_RDONLY );

	if ( file == -1 )
		return;

	broken = false;

	readDirectory ( file );
	close         ( file );

	//printf ( "Added zip source: %s\n", zipName.c_str () );
}
ZipFileStorage :: ~ZipFileStorage ( )
{
}

Data * ZipFileStorage :: getFile  ( const string& name )
{
													// in zip's the '/' is used instead of '\\'
	string	fixedName;

	for ( string :: const_iterator it2 = name.begin (); it2 != name.end (); ++it2 )
		if ( *it2 == '\\' )
			fixedName += '/';
		else
			fixedName += tolower ( *it2 );

	for ( ZipDir :: iterator it = dir.begin (); it != dir.end (); ++it )
		if ( it -> first == fixedName )				// found name
			return readEntry ( it -> second );

	return NULL;
}

bool ZipFileStorage :: hasFile  ( const string& name )
{
													// in zip's the '/' is used instead of '\\'
	string	fixedName;

	for ( string :: const_iterator it2 = name.begin (); it2 != name.end (); ++it2 )
		if ( *it2 == '\\' )
			fixedName += '/';
		else
			fixedName += tolower ( *it2 );

	for ( ZipDir :: iterator it = dir.begin (); it != dir.end (); ++it )
		if ( it -> first == fixedName )				// found name
			return true;

	return false;
}

const std::list<FileInfo>& ZipFileStorage :: getContent(const char_t* location)
{
	mContent.clear();

	for ( ZipDir :: iterator it = dir.begin (); it != dir.end (); ++it )
	{
		FileInfo fileInfo;

		fileInfo.name		= it->first;
		fileInfo.extension	= Path::GetExtension( fileInfo.name );
		fileInfo.path		= Path::Combine( location, fileInfo.name );
		fileInfo.absPath	= Path::Combine( fileName, fileInfo.path);
		fileInfo.isFolder	= false;
		fileInfo.isHidden	= false;
		fileInfo.isReadOnly	= true;
		mContent.push_back(fileInfo);
	}

	return mContent;
}

void	ZipFileStorage :: readDirectory ( int file )
{
	ZipCentralHeader	hdr;
	char				buf [CHUNK_SIZE];
	size_t				step = sizeof ( ZipEndOfCentralDir );

	lseek ( file, 0, SEEK_END );

	size_t	offs    = lseek ( file, 0, SEEK_END );
	size_t	minOffs;
	size_t	pos;

	if ( offs >= (65535 + step ) )
		minOffs = offs - (65535 + step );
	else
		minOffs = 0;

	while ( offs > minOffs )
	{
		if ( offs > sizeof ( buf ) - step )
			offs -= sizeof ( buf ) - step;
		else
			offs = 0;

		lseek ( file, (long)offs, SEEK_SET );

		pos = read ( file, buf, sizeof ( buf ) );

		if ( pos < step )
			continue;

		for ( int i = (int) (pos - step); i > 0; i-- )
			if ( *(dword *) &buf [i] == END_CENTRAL_SIGNATURE )
			{
								// load end of central dir record
				const ZipEndOfCentralDir * dirEndPtr = (const ZipEndOfCentralDir *) &buf [i];

				if ( lseek ( file, dirEndPtr -> centralDirOffset, SEEK_SET ) != dirEndPtr -> centralDirOffset )
				{
					broken = true;

					return;
				}
								// now read central dir structure
				for ( ; ; )
				{
					if ( read ( file, &hdr, sizeof ( hdr ) ) != sizeof ( hdr ) )
						return;				// finished

					if ( hdr.signature != CENTRAL_ZIP_SIGNATURE )
					{
						broken = true;

						return;
					}

							// now read the name
					if ( hdr.filenameLength >= sizeof ( buf ) )
					{
						broken = true;

						return;
					}

					int len = read ( file, buf, hdr.filenameLength );

					buf [len] = '\0';
					
					std::transform(buf, buf+len, buf, ::tolower);

					dir.push_front ( make_pair ( string ( buf ), hdr ) );
				}
			}
	}
}

Data * ZipFileStorage :: readEntry ( const ZipCentralHeader& hdr )
{
	int	size = 0;
	int	file = open ( fileName.c_str (), O_BINARY | O_RDONLY );

	if ( file == -1 )
		return NULL;

	if ( lseek ( file, hdr.relativeLocalHeaderOffset, SEEK_SET ) != hdr.relativeLocalHeaderOffset )
	{
		close ( file );

		return NULL;
	}

	ZipLocalFileHeader	localHdr;

	if ( read ( file, &localHdr, sizeof ( localHdr ) ) != sizeof ( localHdr ) )
	{
		close ( file );

		return NULL;
	}

	if ( localHdr.signature != LOCAL_ZIP_SIGNATURE )
	{
		close ( file );

		return NULL;
	}

	lseek ( file, localHdr.filenameLength + localHdr.extraFieldLength, SEEK_CUR );

	char	inBuffer [2048];
	size_t	blockSize;
	size_t	bytesLeft = hdr.compressedSize;
	void  * buf       = calloc ( 1, hdr.uncompressedSize + 1 );
	int		err       = 0;

	switch ( hdr.compressionMethod )
	{
		case ZIP_STORE:
			if ( read ( file, buf, hdr.compressedSize ) != hdr.compressedSize )
			{
				free  ( buf );
				close ( file );

				return NULL;
			}

			break;

		case ZIP_DEFLATE:
			z_stream	zs;

			memset ( &zs, '\0', sizeof ( zs ) );

			zs.next_out  = (unsigned char *)buf;
			zs.avail_out = hdr.uncompressedSize;
			zs.zalloc    = NULL;
			zs.zfree     = NULL;

			if ( inflateInit2 ( &zs, -DEF_WBITS ) != Z_OK )
			{
				free  ( buf  );
				close ( file );

				return NULL;
			}

			while ( bytesLeft > 0 )
			{
				blockSize   = bytesLeft > sizeof ( inBuffer ) ? sizeof ( inBuffer ) : bytesLeft;
				zs.next_in  = (unsigned char *)inBuffer;
				zs.avail_in = (uint) read ( file, inBuffer, (uint) blockSize );

				err        = inflate ( &zs, bytesLeft >= blockSize ? Z_PARTIAL_FLUSH : Z_FINISH );
				bytesLeft -= blockSize;
			}

			inflateEnd ( &zs );

			if ( err < 0 )
			{
				free  ( buf  );
				close ( file );

				return NULL;
			}

			break;

		default:
			free  ( buf );
			close ( file );

			return NULL;
	}

	close ( file );

	size = hdr.uncompressedSize;

	return new Data ( buf, size );
}

}//namespace FileSystem { 

}//namespace Squirrel {