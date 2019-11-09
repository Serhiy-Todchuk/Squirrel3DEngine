#pragma once

#include "Element.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API Foldout  : public Element
{
	bool mShowFold;
	int mIntendance;

	float mIntendanceSize;

public:
	virtual Element * onLU();
	virtual Element * onLD();
	//return itself to be able to use label as list item
	virtual Element * onRD()		{return this;};
	virtual void draw();
	virtual void resetState();

	bool doesShowFold() { return mShowFold; }
	void setShowFold(bool show) { mShowFold = show; }

	int getIntendance() { return mIntendance; }
	void setIntendance(int intendance) { mIntendance = intendance; }

	Foldout();
	virtual ~Foldout();
};

}//namespace GUI { 
}//namespace Squirrel {