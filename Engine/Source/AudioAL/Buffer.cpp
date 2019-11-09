#include "Buffer.h"
#include "Utils.h"

#ifdef _WIN32
#	include <al/al.h>
#elif __APPLE__
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#endif

namespace Squirrel {
namespace AudioAL { 

Buffer::Buffer(): mId(0)
{
	alGenBuffers(1, &mId);
}

Buffer::~Buffer()
{
	if(alIsBuffer(mId))
		alDeleteBuffers(1, &mId); 
}

bool Buffer::fill(int frequency, int bits, int channels, int size, void *data)
{
	ALenum format = alInternalFormat(bits, channels);

	if(format == 0)
		return false;

	alBufferData(mId, format, data, size, frequency);
	return (!CHECK_AL_ERROR);
}

int Buffer::alInternalFormat(int bits, int channels)
{
	ALenum format = 0;

	if(bits == 8 && channels == 1)
		format = AL_FORMAT_MONO8;
	else if(bits == 8 && channels == 2)
		format = AL_FORMAT_STEREO8;
	else if(bits == 16 && channels == 1)
		format = AL_FORMAT_MONO16;
	else if(bits == 16 && channels == 2)
		format = AL_FORMAT_STEREO16;
	else
	{
		const char_t * channelsStrs[] = {
			NULL,		//0 channels
			"MONO", 	//1 channels
			"STEREO", 	//2 channels
			NULL, 		//3 channels
			"QUAD", 	//4 channels
			NULL, 		//5 channels
			"51CHN", 	//6 channels
			"61CHN", 	//7 channels
			"71CHN" 	//8 channels 
		};

		if(channelsStrs[channels] == NULL)
			return 0;

		char_t str[64];
		sprintf(str, "AL_FORMAT_%s%d", channelsStrs[channels], bits);

#ifdef __APPLE__
		format = alcGetEnumValue(NULL, (const ALCchar *)str);
#else
		format = alGetEnumValue((ALubyte*)str);
#endif
	}

	return format;
}

} //namespace AudioAL {
} //namespace Squirrel {
