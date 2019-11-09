// List.cpp: implementation of the List class.
//
//////////////////////////////////////////////////////////////////////

#include "List.h"
#include "Label.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::List, GUIList);

using namespace Reflection;

const char_t * List::SELECTION_CHANGED_ACTION	= "selectionChanged";
const char_t * List::DOUBLECLICKED_ACTION		= "doubleclicked";
const char_t * List::RIGHTCLICKED_ACTION		= "rightclicked";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

List::List()
{
	SQREFL_SET_CLASS(GUI::List);

	wrapAtomicField("Cell Height", &mCellHeight);

	mContentSource = NULL;
	mPrevRowsNum = 0;

	setSizeSep(60,100);

	mCellHeight = Render::Instance().getFont(Render::sizeNormal)->getCharHeight() + 2;
	mSelected = -1;

	//init default cell creator
	mCellCreator = ObjectCreator::GetCreator("GUI::Label");
	if(mCellCreator == NULL)
	{
		mCellCreator = ObjectCreatorFactory::Create<Label>("GUI::Label");
	}

	addVerticalScroll<Slider>(SCROLL_WIDTH);
	addHorizontalScroll<Slider>(SCROLL_WIDTH);
}

List::~List()
{
	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) != NULL ) delete (*it);
	}
}

tuple2i	List::getContentSize()
{
	tuple2i contentSize(0,0);

	if(mContentSource != NULL)
	{
		int mRowsNum = mContentSource->getRowsNum();
		contentSize.y = mRowsNum * mCellHeight;
		contentSize.x = getSize().x;
	}

	return contentSize;
}

void List::drawContent()
{
	beginDrawContent();

	int firstVisibleElement = getScrollsOffset().y / mCellHeight;
	int visibleElementsNum = (getSize().y / mCellHeight) + 1;

	int mRowsNum = mContentSource->getRowsNum();
	if(mPrevRowsNum != mRowsNum)
	{
		updateScrolls();
		mPrevRowsNum = mRowsNum;
	}

	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(int i = firstVisibleElement; i < firstVisibleElement + visibleElementsNum; ++i)
	{
		if(i >= mRowsNum) break;

		Element * cell = NULL;

		if(it != mElems.end())
		{
			cell = (*it);
			++it;
		}
		else
		{
			Object * cellObj = mCellCreator->create();
			cell = static_cast<Element *>(cellObj);
			mElems.push_back(cell);
			it = mElems.end();
		}

		//setup cell
		mContentSource->setupCell(cell, i);
		cell->setMaster(this);
		cell->setId(i);
		cell->setVisible( true );
		cell->setPosSep(0, i * mCellHeight);
		cell->setSizeSep(getSize().x - (hasVerticalScroll() ? SCROLL_WIDTH : 0), mCellHeight);
		cell->setMasterPos(getGlobalPos() - getScrollsOffset());
		cell->setActionDelegate(this);
		//draw selection
		if(mSelected == i)
		{
			tuple2i cellPos = cell->getGlobalPos();
			tuple2i cellSize= cell->getSize();
			Render::Instance().drawSelectPlane(	cellPos, cellSize );
		}
		cell->draw();
	}
	//hide unnecessary elements
	while(it != mElems.end())
	{
		(*it)->setVisible( false );
		++it;
	}

	endDrawContent();
}

void List::setPos(tuple2i pos)
{
	ScrollView::setPos(pos);
	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		(*it)->setMasterPos(getGlobalPos() - getScrollsOffset());
	}
}

void List::resetStates()
{
	ScrollView::resetState();
	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) == NULL ) continue;
		(*it)->resetState();
	}
}

void List::draw()
{
//	if(mCellHeight == 0)
//		mCellHeight = Render::Instance().getFont(Render::sizeNormal)->getCharHeight() + 2;

	tuple2i p = getGlobalPos();

	//draw background
	if(getDrawStyle() > 0)
	{
		Render::Instance().drawBackPlane(p, getSize());
	}

	//draw content
	drawContent();

	//draw border
	if(getDrawStyle() > 0)
	{
		Render::Instance().drawBorder(p, getSize(), true);
	}
}

void List::select(int cell)
{
	//clamp selected index
	int mRowsNum = mContentSource->getRowsNum();
	mSelected = Math::clamp(cell, 0, mRowsNum - 1);
}

void List::deselect()
{
	mSelected = -1;
}

void List::processEventForCell(EventType e, int value, Element * cell)
{
	if(e == leftDown)
	{
		select( cell->getId() );
		processAction(SELECTION_CHANGED_ACTION, this);
	}
	if(e == rightDown)
	{
		select( cell->getId() );
		processAction(SELECTION_CHANGED_ACTION, this);
		processAction(RIGHTCLICKED_ACTION, this);
	}
	if(e == leftDouble)
	{
		processAction(DOUBLECLICKED_ACTION, this);
	}
}

Element * List::recieveEvent(EventType e, int value)
{
	Element * elem = ScrollView::recieveEvent(e, value);

	tuple2i cursor = Cursor::Instance().getPos();
	if(elem == NULL)
	{
		Container::ELEMENTS_LIST::iterator it = mElems.begin();
		for(; it != mElems.end(); ++it)
		{
			if( (*it) == NULL ) continue;
			if((*it)->getVisible())
			{
				elem = (*it)->recieveEvent(e,value);
				//if(e==mouseMove) updateScrolls();
				if(elem) break;
			}
		}

		if(elem != NULL && elem->getId() >= 0)
		{
			processEventForCell(e, value, elem);
		}
		else if(e == leftDown && isIn(cursor))
		{
			deselect();
			processAction(SELECTION_CHANGED_ACTION, this);
		}
	}

	return elem != NULL ? elem : Element::recieveEvent(e,value);
}

}//namespace GUI { 
}//namespace Squirrel {