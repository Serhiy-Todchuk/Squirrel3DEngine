// DropDownList.cpp: implementation of the DropDownList class.
//
//////////////////////////////////////////////////////////////////////

#include "DropDownList.h"
#include "Label.h"
#include "Render.h"
#include "Cursor.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::DropDownList, GUIDropDownList);

using namespace Reflection;

const char_t * DropDownList::SELECTION_CHANGED_ACTION	= "selectionChanged";
const char_t * DropDownList::DOUBLECLICKED_ACTION		= "doubleclicked";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DropDownList::DropDownList()
{
	SQREFL_SET_CLASS(GUI::DropDownList);

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

DropDownList::~DropDownList()
{
	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) != NULL ) delete (*it);
	}
}

tuple2i	DropDownList::getContentSize()
{
	tuple2i contentSize(0,0);

	if(mContentSource != NULL)
	{
		size_t mRowsNum = mContentSource->getRowsNum();
		contentSize.y = mRowsNum * mCellHeight;
		contentSize.x = getSize().x;
	}

	return contentSize;
}

void DropDownList::drawContent()
{
	beginDrawContent();

	int firstVisibleElement = getScrollsOffset().y / mCellHeight;
	int visibleElementsNum = (getSize().y / mCellHeight) + 1;

	size_t mRowsNum = mContentSource->getRowsNum();
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
			cell = static_cast<Element *>(mCellCreator->create());
			mElems.push_back(cell);
			it = mElems.end();
		}

		//setup cell
		mContentSource->setupCell(cell, i);
		cell->setId(i);
		cell->setVisible( true );
		cell->setPosSep(0, i * mCellHeight);
		cell->setSizeSep(getSize().x - SCROLL_WIDTH, mCellHeight);
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

void DropDownList::setPos(tuple2i pos)
{
	ScrollView::setPos(pos);
	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		(*it)->setMasterPos(getGlobalPos() - getScrollsOffset());
	}
}

void DropDownList::resetStates()
{
	ScrollView::resetState();
	Container::ELEMENTS_LIST::iterator it = mElems.begin();
	for(; it != mElems.end(); ++it)
	{
		if( (*it) == NULL ) continue;
		(*it)->resetState();
	}
}

void DropDownList::draw()
{
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

void DropDownList::select(int cell)
{
	//clamp selected index
	int mRowsNum = mContentSource->getRowsNum();
	mSelected = Math::clamp(cell, 0, mRowsNum - 1);
}

void DropDownList::update()
{

}

Element * DropDownList::recieveEvent(EventType e, int value)
{
	Element * elem = ScrollView::recieveEvent(e, value);

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
	}

	if(elem != NULL && elem->getId() >= 0)
	{
		if(e == leftDown)
		{
			select( elem->getId() );
			processAction(SELECTION_CHANGED_ACTION, this);
		}
		if(e == leftDouble)
		{
			processAction(DOUBLECLICKED_ACTION, this);
		}
	}

	return elem != NULL ? elem : Element::recieveEvent(e,value);
}

}//namespace GUI { 
}//namespace Squirrel {