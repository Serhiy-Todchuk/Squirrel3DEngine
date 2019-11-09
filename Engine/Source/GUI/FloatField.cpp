// FloatField.cpp: implementation of the FloatField class.
//
//////////////////////////////////////////////////////////////////////

#include "FloatField.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

#define VALUE_SCROLLER_WIDTH 8

SQREFL_REGISTER_CLASS_SEED(GUI::FloatField, GUIFloatField);

const float		FloatField::VALUE_SCROLL_STEP		= 0.05f;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FloatField::FloatField()
{
	SQREFL_SET_CLASS(GUI::FloatField);

	wrapAtomicField("Value", &mValue);

	setSizeSep(32, 16);
	mValue = 0;
	mMovingValue = false;

	mLastValueChangeWay = VALUE_CHANGE_WAY::NONE;
}

FloatField::~FloatField()
{

}

void FloatField::draw()
{
	Edit::draw();

	//draw scroller
	if(getDrawStyle() > 0)
	{
		tuple2i pos = getGlobalPos();
		pos.x += getSize().x - VALUE_SCROLLER_WIDTH;

		tuple2i size = getSize();
		size.x = VALUE_SCROLLER_WIDTH;

		Render::Instance().drawPlane( pos, size, mMovingValue);
		Render::Instance().drawBorder(pos, size, mMovingValue);
	}
}

Element * FloatField::onLU()
{
	Edit::onLU();
	Cursor::Instance().setScroll(false);
	mMovingValue = false;
	return this;
}

Element * FloatField::onMM()
{
	if(mMovingValue)
	{
		tuple2i prevMouse = Cursor::Instance().getPrevPos();
		tuple2i currMouse = Cursor::Instance().getPos();
		int yDiff = currMouse.y - prevMouse.y;
		float valueScrollIncrement = 1.0f;//Math::maxValue(abs(mValueBeforeScroll - mValue), 1.0f);
		mLastValueChangeWay = VALUE_CHANGE_WAY::SCROLLED;
		setValue_internal( mValue - (VALUE_SCROLL_STEP * valueScrollIncrement * yDiff) );
		return this;
	}
	else
	{
		return Edit::onMM();
	}
	return NULL;
}

void FloatField::resetState()
{
	Edit::resetState();
	Cursor::Instance().setScroll(false);
	mMovingValue = false;
}

Element * FloatField::onLD()
{
	tuple2i scrollerPos = getGlobalPos();
	scrollerPos.x += getSize().x - VALUE_SCROLLER_WIDTH;

	tuple2i scrollerSize = getSize();
	scrollerSize.x = VALUE_SCROLLER_WIDTH;

	tuple2i mousePos = Cursor::Instance().getPos();
	if(IsIn(scrollerPos, scrollerPos + scrollerSize, mousePos))
	{
		mMovingValue = true;
		Cursor::Instance().setScroll(true);
		mValueBeforeScroll = mValue;
		return this;
	}
	else
	{
		return Edit::onLD();
	}
	return NULL;
}

void FloatField::textEditEnded()
{
	float newValue = static_cast<float>( atof( getText().c_str() ) );
	mLastValueChangeWay = VALUE_CHANGE_WAY::EDITED;
	setValue_internal( newValue );
}

int calcNumberOfDigitsAfterDot(float value, int bound = 5)
{
	const int buffSize = 80;
	char strBuffer[ buffSize ];
	sprintf(strBuffer, "%1.5f", value);

	int result = 0;
	bool searchDecimalPoint = false;
	for(int i = 0; i < buffSize; ++i)
	{
		if(searchDecimalPoint)
		{
			if(strBuffer[i] == '.')
				searchDecimalPoint = false;
		}
		else
		{
			if(strBuffer[i] != '0')
			{
				if(++result >= bound) return result;
			}
		}
	}

	return result;
}

void FloatField::setValue(float value)
{
	mLastValueChangeWay = VALUE_CHANGE_WAY::EXTERNAL;

	setValue_internal(value);
}

void FloatField::setValue_internal(float value)
{
	if(value != mValue)
	{
		mValue = value;

		int precision = 3;//calcNumberOfDigitsAfterDot(mValue);
		if(precision == 0) precision = 1;

		char formatBuffer[32];
		sprintf(formatBuffer, "%%1.%df", precision);

		char strBuffer[32];
		sprintf(strBuffer, formatBuffer, mValue);

		setText(strBuffer);

		processAction(VALUE_CHANGED_ACTION, this);
	}
}

Element * FloatField::onCh(char_t ch)
{
	if(	(ch >= '0' && ch <= '9') ||
		(ch == '.' || ch == '-') ||
		(ch == '+') )
	{
		Edit::onCh(ch);
	}
	return this;
}

}//namespace GUI { 
}//namespace Squirrel {