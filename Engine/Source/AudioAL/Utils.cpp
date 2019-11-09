// Utils.cpp: implementation of the Utils class.
//
//////////////////////////////////////////////////////////////////////
#include "Utils.h"
#include <Math/BasicUtils.h>
#include <Common/Log.h>

#ifdef _WIN32
#	include <al/al.h>
#elif __APPLE__
#	include <OpenAL/al.h>
#endif

namespace Squirrel {

namespace AudioAL {

bool Utils::CheckALError(const char_t * msg)
{
	ALenum errCode = alGetError();
	if(errCode != AL_NO_ERROR)
	{
		const char_t * str = (const char_t *)alGetString(errCode);
		Log::Instance().streamError("CheckALError") << msg << " " << str;
		Log::Instance().flush();
		//MessageBox(NULL, str, msg, MB_OK);
		return true;
	}
	return false;
}

} //namespace AudioAL {

} //namespace Squirrel {
