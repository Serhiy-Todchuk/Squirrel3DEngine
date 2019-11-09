#pragma once

#include "Container.h"
#include "List.h"

namespace Squirrel {
namespace GUI {

class SQGUI_API DropDownList  : public ScrollView
{
public:

	static const char_t * SELECTION_CHANGED_ACTION;
	static const char_t * DOUBLECLICKED_ACTION;

public:
	DropDownList();
	virtual ~DropDownList();

	Reflection::ObjectCreator *	getCellCreator() { return mCellCreator; }
	void setCellCreator(Reflection::ObjectCreator * cellCreator) { ASSERT(mCellCreator = cellCreator); }

	ListContentSource * getContentSource() { return mContentSource; }
	void setContentSource(ListContentSource * contentSource) { ASSERT(mContentSource = contentSource); }

	int getSelected() { return mSelected; }
	void select(int cell);
	void update();

	virtual void draw();

	virtual void setPos(tuple2i pos);

	virtual Element * recieveEvent(EventType e, int value);

	virtual void resetStates();

protected:

	virtual tuple2i		getContentSize();
	virtual void		drawContent();

private:

	Container::ELEMENTS_LIST mElems;
	int mPrevRowsNum;

protected:

	Reflection::ObjectCreator *		mCellCreator;
	int								mSelected;
	ListContentSource *				mContentSource;
	int								mCellHeight;
};



}//namespace GUI { 
}//namespace Squirrel {