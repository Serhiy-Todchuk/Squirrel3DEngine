//
// Basic class to handle reading of static data
//

#pragma once

#include	"macros.h"
#include	"Types.h"
#include	<stdarg.h>
#include	<string>

#ifdef	_WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
# endif
# include	<windows.h>
#endif

namespace Squirrel {

class SQCOMMON_API Data
{
private:
	byte  *		mBits;
	size_t		mCapacityIncrement;//usefull for writing when actual size is unknown
	size_t		mCapacity;
	size_t		mLength;
	size_t		mPos;

	std::string	mFileName;
	int			mVersion;

#ifdef	_WIN32
	void *		mMappedFile;
	void *		mFileMapping;
#else
	int			mMappedFile;
#endif

private:

	void initMembers();
	int readFile ( const char_t * fileName );

	void realloc(size_t newLen);
	void queryCapacity(size_t bytesNum);

public:
	Data ( const char_t * fileName );
	Data ( const char_t * fileName, bool mapFile, bool readOnly );
	Data ( void * ptr, size_t len );
	virtual ~Data ();

	size_t writeToFile ( const char_t * fileName ); 

	bool	isOk () const;

	inline void	setCapacityIncrement (size_t v) { mCapacityIncrement = v; };
	inline void	setVersion (int v) { mVersion = v; };

	inline bool	isEmpty () const	{	return mPos >= mLength;	}
	inline bool	isMappedFile() const { return mMappedFile != 0; }

	inline int	getVersion () { return mVersion; };
	inline const std::string&	getFileName () const	{	return mFileName;	}
	
	inline size_t	getLength () const	{	return mLength;	}
	inline size_t	getPos ()	const	{	return mPos;	}

	inline void * getData() const { return (void *)mBits; }

	void * getPtr () const;
	void * getPtr ( int offs ) const;

	byte	readByte ();
	int16	readInt16 ()	{ return readVar<int16>(); }
	uint16	readUInt16 ()	{ return readVar<uint16>(); }
	int32	readInt32 ()	{ return readVar<int32>(); }
	uint32	readUInt32 ()	{ return readVar<uint32>(); }
	int64	readInt64 ()	{ return readVar<int64>(); }
	uint64	readUInt64 ()	{ return readVar<uint64>(); }
	float	readFloat ()	{ return readVar<float>(); }
	double	readDouble ()	{ return readVar<double>(); }

	size_t		readBytes  ( void * ptr, size_t len );
	bool		readString ( std::string& str, char_t term );				// get a string with given terminator
	std::string readString();

	int		seekCur ( int delta );
	int		seekAbs ( int offs );

	int		seekUntil(char_t term);
	int		seekSkipAnyOf(const char_t * chars);

	void	putData (const void * data, size_t dataLen);
	void	putString (const std::string& str);
	void	putByte (byte b);
	void	putInt16 (int16 s)		{ putVar<int16>(s); }
	void	putUInt16 (uint16 s)	{ putVar<uint16>(s); }
	void	putInt32 (int32 s)		{ putVar<int32>(s); }
	void	putUInt32 (uint32 s)	{ putVar<uint32>(s); }
	void	putInt64 (int64 s)		{ putVar<int64>(s); }
	void	putUInt64 (uint64 s)	{ putVar<uint64>(s); }
	void	putFloat (float s)		{ putVar<float>(s); }

	template<class _T>
	_T readVar ()
	{
		int typeSize = sizeof(_T);

		if ( mPos + typeSize > mLength )
			return _T();

		_T 	v = *(_T *) (mBits + mPos);

		mPos += typeSize;

		return v;
	}

	template<class _T>
	void putVar (_T s)
	{
		int tsize = sizeof(_T);
		queryCapacity( tsize );
		(*(_T *) (mBits + mPos)) = s;
		mPos += tsize;
	}
};

}//namespace Squirrel {

