#pragma once

#include "Container.h"

namespace Squirrel {
namespace GUI {

class SQGUI_API ListContentSource
{
public:
	ListContentSource() {}
	virtual ~ListContentSource() {}

	virtual size_t		getRowsNum()						= 0;
	virtual void		setupCell(Element * cell, int row)	= 0;
};

class SQGUI_API List  : public ScrollView
{
public:

	static const char_t * SELECTION_CHANGED_ACTION;
	static const char_t * DOUBLECLICKED_ACTION;
	static const char_t * RIGHTCLICKED_ACTION;

public:
	List();
	virtual ~List();

	Reflection::ObjectCreator *	getCellCreator() { return mCellCreator; }
	void setCellCreator(Reflection::ObjectCreator * cellCreator) { 
		mCellCreator = cellCreator; 
	}

	ListContentSource * getContentSource() { return mContentSource; }
	void setContentSource(ListContentSource * contentSource) { 		
		//DELETE_PTR(mContentSource); 
		ASSERT(mContentSource = contentSource); 
	}

	int getSelected() { return mSelected; }
	void select(int cell);
	void deselect();

	virtual void draw();

	virtual void setPos(tuple2i pos);

	virtual Element * recieveEvent(EventType e, int value);

	virtual void resetStates();

protected:

	virtual tuple2i		getContentSize();
	virtual void		drawContent();

	virtual void processEventForCell(EventType e, int value, Element * cell);

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