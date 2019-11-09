#pragma once

#include "Edit.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API IntField  : public Edit
{
public:

	static const char_t * VALUE_CHANGED_ACTION;
	static const int VALUE_SCROLL_STEP;

private:

	int		mValue;
	int		mValueBeforeScroll;
	bool	mMovingValue;

	tuple2i mValueRange;

public:
	IntField();
	virtual ~IntField();

	inline tuple2i getValueRange() { return mValueRange; }
	void setValueRange(tuple2i valueRange) { mValueRange = valueRange; }

	inline int getValue() { return mValue; }
	void setValue(int value);

	virtual Element * onLU();
	virtual Element * onLD();
	virtual Element * onMM();
	virtual Element * onCh(char ch);
	virtual void draw();
	virtual void resetState();

protected:
	virtual void textEditEnded();
};
}//namespace GUI { 
}//namespace Squirrel {