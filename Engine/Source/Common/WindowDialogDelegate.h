#pragma once

#include "macros.h"

namespace Squirrel {

class SQCOMMON_API WindowDialogDelegate
{
public:
	
	enum EAnswer
	{
		answerYes = 0,
		answerNo = 1,
	};
	
public:
	WindowDialogDelegate() {};
	virtual ~WindowDialogDelegate() {};
	
	virtual void windowDialogEnd(int answer, void * contextInfo) = 0;
};

}//namespace Squirrel {