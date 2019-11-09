// Switch.h: interface for the Switch class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Button.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Switch  : public Button
{
public:
	virtual Element * onLU();
	virtual Element * onRU();
	virtual Element * onLD();
	virtual Element * onRD();

	Switch();
	virtual ~Switch();

	virtual void resetState();
};

}//namespace GUI { 
}//namespace Squirrel {