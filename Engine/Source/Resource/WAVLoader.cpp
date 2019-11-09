#include "WAVLoader.h"

namespace Squirrel {

namespace Resource { 

#pragma pack(push, 4)

#ifndef GUID
	typedef struct _GUID {
		unsigned int	Data1;
		unsigned short	Data2;
		unsigned short	Data3;
		unsigned char	Data4[ 8 ];
	} GUID;
#endif
	
typedef struct
{
	char			szRIFF[4];
	unsigned int	ulRIFFSize;
	char			szWAVE[4];
} WAVEFILEHEADER;

typedef struct
{
	char			szChunkName[4];
	unsigned int	ulChunkSize;
} RIFFCHUNK;

typedef struct
{
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned int	ulSamplesPerSec;
	unsigned int	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned int	ulChannelMask;
    GUID            guidSubFormat;
} WAVEFMT;

#pragma pack(pop)

WAVLoader::WAVLoader() {}

WAVLoader::~WAVLoader() {		
	if(isOpened())
		close();
}

bool WAVLoader::open(Data * srcData)
{
	if(!srcData)
		return false;

	mSrcData = srcData;

	WAVEFILEHEADER	waveFileHeader;
	RIFFCHUNK		riffChunk;
	WAVEFMT			waveFmt;

	mDataSize	= 0;
	mDataOffset	= 0;

	// Read Wave file header
	mSrcData->readBytes(&waveFileHeader, sizeof(WAVEFILEHEADER));

	for(int i = 0; i < 4; ++i)
	{
		waveFileHeader.szRIFF[i] = toupper(waveFileHeader.szRIFF[i]);
		waveFileHeader.szWAVE[i] = toupper(waveFileHeader.szWAVE[i]);
	}
	
	if (strncmp(waveFileHeader.szRIFF, "RIFF", 4) || strncmp(waveFileHeader.szWAVE, "WAVE", 4))
	{
		close();
		return false;
	}

	while (mSrcData->readBytes(&riffChunk, sizeof(RIFFCHUNK)) == sizeof(RIFFCHUNK))
	{
		if (!strncmp(riffChunk.szChunkName, "fmt ", 4))
		{
			if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
			{
				mSrcData->readBytes(&waveFmt, riffChunk.ulChunkSize);
			}
			else
			{
				mSrcData->seekCur(riffChunk.ulChunkSize);
			}
		}
		else if (!strncmp(riffChunk.szChunkName, "data", 4))
		{
			mDataSize = riffChunk.ulChunkSize;
			mDataOffset = srcData->getPos();
			mSrcData->seekCur(riffChunk.ulChunkSize);
		}
		else
		{
			mSrcData->seekCur(riffChunk.ulChunkSize);
		}

		// Ensure that we are correctly aligned for next chunk
		if (riffChunk.ulChunkSize & 1)
			mSrcData->seekCur(1);
	}

	//check 
	if(!mDataOffset || !mDataSize)
	{
		close();
		return false;
	}

	mFrequency		= waveFmt.ulSamplesPerSec;
	mBitsPerSample	= waveFmt.usBitsPerSample;
	mChannels		= waveFmt.usChannels;

	const uint32 bytesPerSample	= mBitsPerSample / 8;
	uint64 bytesPerSecond = (uint64)mFrequency * bytesPerSample * mChannels;

	mDuration		= (double)mDataSize / bytesPerSecond;

	return true;
}

bool WAVLoader::load(Audio::IBuffer * targetBuffer, double offset, double duration)
{
	const uint32 bytesPerSample	= mBitsPerSample / 8;

	uint32 bufferSize = mFrequency * bytesPerSample * mChannels * duration;
	uint32 bufferOffset = mFrequency * bytesPerSample * mChannels * offset;
	if(duration <= 0)
	{
		bufferSize		= mDataSize - bufferOffset;
	}
	// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
	//bufferSize -= (bufferSize % (bytesPerSample * mChannels));

	//read data
	return targetBuffer->fill( 
		mFrequency, mBitsPerSample, mChannels, 
		mDataOffset + bufferSize, 
		mSrcData->getPtr( bufferOffset ) );
}

void WAVLoader::close()
{
	mSrcData = NULL;
}

}//namespace Resource { 

}//namespace Squirrel {