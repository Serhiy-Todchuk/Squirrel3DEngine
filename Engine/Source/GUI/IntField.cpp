// IntField.cpp: implementation of the IntField class.
//
//////////////////////////////////////////////////////////////////////

#include "IntField.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

#define VALUE_SCROLLER_WIDTH 8

SQREFL_REGISTER_CLASS_SEED(GUI::IntField, GUIIntField);

const char_t *	IntField::VALUE_CHANGED_ACTION	= "valueChanged";
const int		IntField::VALUE_SCROLL_STEP		= 1;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IntField::IntField(): 
	mValue(-MAX_COORD), mMovingValue(false), mValueRange(-MAX_COORD, +MAX_COORD)
{
	SQREFL_SET_CLASS(GUI::IntField);

	wrapAtomicField("Value", &mValue);

	setSizeSep(32, 16);	
}

IntField::~IntField()
{

}

void IntField::draw()
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

Element * IntField::onLU()
{
	Edit::onLU();
	Cursor::Instance().setScroll(false);
	mMovingValue = false;
	return this;
}

Element * IntField::onMM()
{
	if(mMovingValue)
	{
		tuple2i prevMouse = Cursor::Instance().getPrevPos();
		tuple2i currMouse = Cursor::Instance().getPos();
		int yDiff = currMouse.y - prevMouse.y;
		setValue( mValue - (VALUE_SCROLL_STEP * yDiff) );
	}
	else
	{
		Edit::onMM();
	}
	return this;
}

void IntField::resetState()
{
	Edit::resetState();
	Cursor::Instance().setScroll(false);
	mMovingValue = false;
}

Element * IntField::onLD()
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
	}
	else
	{
		Edit::onLD();
	}
	return this;
}

void IntField::textEditEnded()
{
	int newValue = static_cast<int>( atoi( getText().c_str() ) );
	setValue( newValue );
}

void IntField::setValue(int value)
{
	value = Math::clamp(value, mValueRange.x, mValueRange.y);
	if(value != mValue)
	{
		mValue = value;

		char strBuffer[32];
		sprintf(strBuffer, "%d", mValue);

		setText(strBuffer);

		processAction(VALUE_CHANGED_ACTION, this);
	}
}

Element * IntField::onCh(char_t ch)
{
	if(	(ch >= '0' && ch <= '9') ||
		(ch == '-') ||
		(ch == '+') )
	{
		Edit::onCh(ch);
	}
	return this;
}

}//namespace GUI { 
}//namespace Squirrel {