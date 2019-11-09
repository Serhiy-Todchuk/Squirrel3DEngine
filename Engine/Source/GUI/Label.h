#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Label  : public Element
{
public:
	Label();
	virtual ~Label();

	virtual void draw();

	//return itself to be able to use label as list item
	virtual Element * onLU()		{return this;};
	virtual Element * onLD()		{return this;};
	virtual Element * onDBL()		{return this;};
	virtual Element * onRD()		{return this;};

	virtual Element * recieveEvent(EventType e, int value);
};

}//namespace GUI { 
}//namespace Squirrel {