#pragma once

#include <Audio/IBuffer.h>
#include <common/types.h>
#include "macros.h"

namespace Squirrel {

namespace AudioAL { 
	
using namespace Audio;

class SQOPENAL_API Buffer:
	public IBuffer
{
public://ctor/destr

	Buffer();
	virtual ~Buffer();

public: //methods

	virtual bool fill(int frequency, int bits, int channels, int size, void *data);

	uint getId() { return mId; }

private:

	uint mId;

	static int alInternalFormat(int bits, int channels);

};


} //namespace AudioAL {

} //namespace Squirrel {
