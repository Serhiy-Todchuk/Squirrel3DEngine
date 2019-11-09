#pragma once

#include "Panel.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Window  : public Panel
{
	float mHeaderHeight;

public:
	Window();
	virtual ~Window();

	virtual void draw();

	virtual Element * onLD();
	virtual Element * onMM();

protected:

	//hide changing margin from outside
	void setMargin(tuple4i margin) { mMargin = margin; }
};

}//namespace GUI { 
}//namespace Squirrel {