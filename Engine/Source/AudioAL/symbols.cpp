#include <AudioAL/Audio.h>

extern "C" SQOPENAL_API Squirrel::Audio::IAudio * CreateAudio()
{
	return new Squirrel::AudioAL::Audio();
}

extern "C" SQOPENAL_API void DestroyAudio(Squirrel::Audio::IAudio * render)
{
	if(render)
		delete render;
}

extern "C" SQOPENAL_API const char_t * GetAudioName()
{
	return "OpenAL 1.1 renderer";
}
