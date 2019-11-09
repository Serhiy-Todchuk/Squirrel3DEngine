#pragma once

#include "macros.h"
#include "types.h"

namespace Squirrel {

class SQCOMMON_API Clipboard
{
protected:
	Clipboard() {};
public:
	virtual ~Clipboard() {};

	virtual void setText(const char_t * str) = 0;
	virtual std::string getText() = 0;

	virtual bool isTextAvailable() = 0;
};

}//namespace Squirrel {