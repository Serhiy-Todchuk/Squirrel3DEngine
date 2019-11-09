#pragma once

#include "macros.h"

namespace Squirrel {

namespace AudioAL {

#ifdef _DEBUG
#	define CHECK_AL_ERROR		AudioAL::Utils::CheckALError( ("ALError at " TOSTRING(__LINE__) ", file " __FILE__) )
#else
#	define CHECK_AL_ERROR		false
#endif

class SQOPENAL_API Utils  
{
public:
	
	static bool CheckALError(const char * msg);

};

} //namespace AudioAL {

} //namespace Squirrel {
