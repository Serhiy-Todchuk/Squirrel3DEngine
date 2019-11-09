#pragma once

#include "../Clipboard.h"

namespace Squirrel {

class SQCOMMON_API MacClipboard: 
	public Clipboard
{
public:
	MacClipboard();
	virtual ~MacClipboard();

	virtual void setText(const char_t * str);
	virtual std::string getText();

	virtual bool isTextAvailable();
};

}//namespace Squirrel {