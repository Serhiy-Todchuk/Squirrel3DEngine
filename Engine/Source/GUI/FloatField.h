#pragma once

#include "Edit.h"

namespace Squirrel {
namespace GUI { 

class SQGUI_API FloatField  : public Edit
{
public:

	static const float VALUE_SCROLL_STEP;

	//the way value changed
	enum VALUE_CHANGE_WAY
	{
		NONE,
		EDITED,
		SCROLLED,
		EXTERNAL,
	};

private:

	float	mValue;
	float	mValueBeforeScroll;
	bool	mMovingValue;

	VALUE_CHANGE_WAY mLastValueChangeWay;

public:
	FloatField();
	virtual ~FloatField();

	inline float getValue() { return mValue; }
	void setValue(float value);

	inline VALUE_CHANGE_WAY getLastValueChangeWay() { return mLastValueChangeWay; }

	virtual Element * onLU();
	virtual Element * onLD();
	virtual Element * onMM();
	virtual Element * onCh(char ch);
	virtual void draw();
	virtual void resetState();

protected:
	virtual void textEditEnded();

	void setValue_internal(float value);
};
}//namespace GUI { 
}//namespace Squirrel {