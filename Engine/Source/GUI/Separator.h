
#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Separator  : public Element
{
public:
	Separator();
	virtual ~Separator();

	virtual void draw();
};

}//namespace GUI { 
}//namespace Squirrel {