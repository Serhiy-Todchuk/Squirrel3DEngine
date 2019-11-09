#pragma once

#include "../common/macros.h"

namespace Squirrel {

namespace Audio { 

class SQCOMMON_API IBuffer
{
public: //nested types

public://ctor/destr

	IBuffer() {};
	virtual ~IBuffer() {};

public: //methods

	virtual bool fill(int frequency, int bits, int channels, int size, void *data) = 0;

};


} //namespace Audio {

} //namespace Squirrel {
