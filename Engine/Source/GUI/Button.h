#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Button  : public Element
{
public:

	static const char_t * LEFT_CLICK_ACTION;

	virtual Element * onLU();
	virtual Element * onRU();
	virtual Element * onLD();
	virtual Element * onRD();
	virtual Element * onMM();
	virtual void draw();

	Button();
	virtual ~Button();
};

}//namespace GUI { 
}//namespace Squirrel {