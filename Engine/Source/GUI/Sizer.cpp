// FileBrowser.cpp: implementation of the FileBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include "Sizer.h"
#include "Render.h"
#include "Cursor.h"
#include "Container.h"

namespace Squirrel {
namespace GUI { 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Sizer::Sizer()
{
	SQREFL_SET_CLASS(GUI::Sizer);

	direction = southEast;
	setSizeSep(6,6);
	mMinSize = tuple2i(6,6);
	setDragable(1);
}

Sizer::~Sizer()
{

}

Element * Sizer::onLU()
{
	setState(Element::stateStd);
	return this;
}

Element * Sizer::onLD()
{
	if(getDragable())
	{
		setState(Element::stateActive);
		mClickedPos = Cursor::Instance().getPos();
		if(getMaster())
		{
			mClickedSize = getMaster()->getSize();
		}
	}
	return this;
}

Element * Sizer::onMM()
{
	tuple2i cursor = Cursor::Instance().getPos();
	if(isIn(cursor))
	{
		Cursor::Instance().setAppearance(Cursor::tSizeNWSE);
		return NULL;
	}
	if(getMaster() && getState() == Element::stateActive)
	{
		tuple2i diff = Cursor::Instance().getPos() - mClickedPos;

		tuple2i newSize = mClickedSize + diff;

		if(newSize.x < mMinSize.x) newSize.x = mMinSize.x;
		if(newSize.y < mMinSize.y) newSize.y = mMinSize.y;

		tuple2i newPos = getPos() + (newSize - mClickedSize);//+diff

		setPos(newPos);
		getMaster()->setSize(newSize);
		return this;
	}
	return NULL;
}

void Sizer::draw()
{
	Element::draw();
}

}//namespace GUI { 
}//namespace Squirrel {