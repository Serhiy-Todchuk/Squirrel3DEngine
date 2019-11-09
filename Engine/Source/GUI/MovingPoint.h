
#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API MovingPoint: public Element
{
public:

	static const char_t * MOVING_ACTION;

public:
	MovingPoint();
	virtual ~MovingPoint();

	inline tuple2i getPrevPos() { return mPrevPos; }

	virtual Element * onLU();
	virtual Element * onLD();
	virtual Element * onMM();

private:
	tuple2i mPrevPos;
};

}//namespace GUI { 
}//namespace Squirrel {