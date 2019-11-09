#include "OGGLoader.h"
#include <Common/DynamicLibrary.h>
#include <Vorbis/vorbisfile.h>

namespace Squirrel {

// Function pointers
typedef int				(*LPOVCLEAR)			(OggVorbis_File *vf);
typedef long			(*LPOVREAD)				(OggVorbis_File *vf,char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream);
typedef ogg_int64_t		(*LPOVPCMTOTAL)			(OggVorbis_File *vf,int i);
typedef vorbis_info *	(*LPOVINFO)				(OggVorbis_File *vf,int link);
typedef vorbis_comment *(*LPOVCOMMENT)			(OggVorbis_File *vf,int link);
typedef int				(*LPOVOPENCALLBACKS)	(void *datasource, OggVorbis_File *vf,char *initial, long ibytes, ov_callbacks callbacks);
typedef long			(*LPov_seekable)		(OggVorbis_File *vf);
typedef double			(*LPov_time_total)		(OggVorbis_File *vf,int i);
typedef int				(*LPov_time_seek)		(OggVorbis_File *vf,double pos);

// Variables
LPOVCLEAR			_ov_clear			= NULL;
LPOVREAD			_ov_read			= NULL;
LPOVPCMTOTAL		_ov_pcm_total		= NULL;
LPOVINFO			_ov_info			= NULL;
LPOVCOMMENT			_ov_comment			= NULL;
LPOVOPENCALLBACKS	_ov_open_callbacks	= NULL;
LPov_seekable		_ov_seekable		= NULL;
LPov_time_total		_ov_time_total		= NULL;
LPov_time_seek		_ov_time_seek		= NULL;

DynamicLibrary sVorbisfileModule;

namespace Resource { 

#define TEMP_BUFF_SIZE			512000
#define MAX_TEMP_BUFFS_NUM		1024

size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return ((Data *)datasource)->readBytes(ptr, size * nmemb);
}

int ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	Data *source = (Data *)datasource;
	int integer_offert=(int)offset;
	int returnVal;

	switch(whence)
	{
	case SEEK_SET:
		returnVal = source->seekAbs(integer_offert);
		break;
	case SEEK_CUR:
		returnVal = source->seekCur(integer_offert);
		break;
	case SEEK_END:
		returnVal = source->seekAbs(source->getLength() - integer_offert);
		break;
	default:		//Bad value
		return -1;
	}

	//if(returnVal > 0)
		return 0;
	//else
	//	return -1;		//Could not do a seek. Device not capable of seeking. (Should never encounter this case)
}

int ov_close_func(void *datasource)
{
   return 0;
}

long ov_tell_func(void *datasource)
{
	return ((Data *)datasource)->getPos();
}

void swap(short &s1, short &s2)
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}

uint32 decodeOggVorbis(OggVorbis_File *psOggVorbisFile, char *pDecodeBuffer, uint32 ulBufferSize, uint32 ulChannels)
{
	int current_section;
	int32 lDecodeSize;
	uint32 ulSamples;
	int16 *pSamples;

	uint32 ulBytesDone = 0;
	while (1)
	{
		lDecodeSize = _ov_read(psOggVorbisFile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, &current_section);
		if (lDecodeSize > 0)
		{
			ulBytesDone += lDecodeSize;

			if (ulBytesDone >= ulBufferSize)
				break;
		}
		else
		{
			break;
		}
	}

	// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
	// however 6-Channels files need to be re-ordered
	if (ulChannels == 6)
	{		
		pSamples = (int16*)pDecodeBuffer;
		for (ulSamples = 0; ulSamples < (ulBufferSize>>1); ulSamples+=6)
		{
			// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
			// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
			swap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
			swap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
			swap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
		}
	}

	return ulBytesDone;
}


OGGLoader::OGGLoader(): 
	mOVFile(NULL), mBuffer(NULL), mBufferSize(0) 
{
	mOVFile = new OggVorbis_File;
	memset(mOVFile, 0, sizeof(OggVorbis_File));
}

OGGLoader::~OGGLoader() 
{
	if(isOpened())
		close();
	DELETE_ARR(mBuffer);
	DELETE_PTR(mOVFile);
}

bool OGGLoader::open(Data * srcData)
{
	if(!sVorbisfileModule.isOpened())
	{
		// vorbisfile.dll will load ogg.dll and vorbis.dll
		if(!sVorbisfileModule.open("vorbisfile.dll"))
		{
			return false;
		}

		_ov_clear			= (LPOVCLEAR		)sVorbisfileModule.getSymbol("ov_clear");
		_ov_read			= (LPOVREAD			)sVorbisfileModule.getSymbol("ov_read");
		_ov_pcm_total		= (LPOVPCMTOTAL		)sVorbisfileModule.getSymbol("ov_pcm_total");
		_ov_info			= (LPOVINFO			)sVorbisfileModule.getSymbol("ov_info");
		_ov_comment			= (LPOVCOMMENT		)sVorbisfileModule.getSymbol("ov_comment");
		_ov_open_callbacks	= (LPOVOPENCALLBACKS)sVorbisfileModule.getSymbol("ov_open_callbacks");
		_ov_seekable		= (LPov_seekable	)sVorbisfileModule.getSymbol("ov_seekable");
		_ov_time_total		= (LPov_time_total	)sVorbisfileModule.getSymbol("ov_time_total");
		_ov_time_seek		= (LPov_time_seek	)sVorbisfileModule.getSymbol("ov_time_seek");
	}

	mSrcData = srcData;

	// Open Ogg Stream
	ov_callbacks	sCallbacks;
	vorbis_info		*psVorbisInfo;

	sCallbacks.read_func = ov_read_func;
	sCallbacks.seek_func = ov_seek_func;
	sCallbacks.close_func = ov_close_func;
	sCallbacks.tell_func = ov_tell_func;

	mFrequency = 0;
	mChannels = 0;
	mBitsPerSample = 16;
	mDuration = 0;

	mSrcData->seekAbs(0);

	// Create an OggVorbis file stream
	int code = _ov_open_callbacks(mSrcData, mOVFile, NULL, 0, sCallbacks);
	if (code != 0)
	{
		close();
		return false;
	}

	// Get some information about the file (Channels, Format, and Frequency)
	psVorbisInfo = _ov_info(mOVFile, -1);
	if (!psVorbisInfo)
	{
		close();
		return false;
	}

	mFrequency = psVorbisInfo->rate;
	mChannels = psVorbisInfo->channels;

	mIsSeekable = _ov_seekable(mOVFile) != 0;
	if(mIsSeekable)
	{
		mDuration = _ov_time_total(mOVFile, -1);
	}
	else
	{
		mDuration = 0;
	}

	// Set BufferSize to 250ms (Frequency * bytesPerSample(16bit) * channelsNum divided by 4 (quarter of a second))
	//bufferSize = frequency * bytesPerSample * channels / 4;
	// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
	//bufferSize -= (bufferSize % (bytesPerSample * channels));

	return true;
}

bool OGGLoader::load(Audio::IBuffer * targetBuffer, double offset, double duration)
{
	const uint32 bytesPerSample	= mBitsPerSample / 8;

	uint32 bytesWritten = 0;

	if(duration <= 0)
	{
		char * buffers[MAX_TEMP_BUFFS_NUM];
		int buffersNum = 0, i = 0;

		while(1)
		{
			i = buffersNum;
			buffers[i] = new char[TEMP_BUFF_SIZE];

			uint32 lBytesWritten = decodeOggVorbis(mOVFile, buffers[i], TEMP_BUFF_SIZE, mChannels);
			if(lBytesWritten == 0)
				break;

			bytesWritten += lBytesWritten;
			buffersNum++;
		}
		
		if(bytesWritten == 0)
			return false;

		if(mBufferSize < bytesWritten)
		{
			DELETE_ARR(mBuffer);
			mBuffer = new char[bytesWritten];
			mBufferSize = bytesWritten;
		}

		int bufferOffset = 0;
		for(i = 0; i < buffersNum; ++i)
		{
			int bytesLeft = bytesWritten - bufferOffset;
			memcpy(mBuffer + bufferOffset, buffers[i], bytesLeft < TEMP_BUFF_SIZE ? bytesLeft : TEMP_BUFF_SIZE);
			bufferOffset += TEMP_BUFF_SIZE;
		}
	}
	else
	{
		// Set BufferSize to 250ms (Frequency * bytesPerSample(16bit) * channelsNum divided by 4 (quarter of a second))
		uint64 bufferSize = (uint64)mFrequency * bytesPerSample * mChannels * duration;
		// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
		bufferSize -= (bufferSize % (bytesPerSample * mChannels));

		if(mBufferSize < bufferSize)
		{
			DELETE_ARR(mBuffer);
			mBuffer = new char[bufferSize];
			mBufferSize = bufferSize;
		}

		if(!mBuffer)
			return false;

		if(mIsSeekable)
		{
			_ov_time_seek(mOVFile, offset);
		}

		bytesWritten = decodeOggVorbis(mOVFile, mBuffer, bufferSize, mChannels);
		if (!bytesWritten)
			return false;
	}

	return targetBuffer->fill(mFrequency, mBitsPerSample, mChannels, bytesWritten, mBuffer);
}

void OGGLoader::close()
{
	// Close OggVorbis stream
	if(mOVFile)
	{
		_ov_clear(mOVFile);
		mOVFile = NULL;
	}

	mSrcData = NULL;
}

}//namespace Resource { 

}//namespace Squirrel {