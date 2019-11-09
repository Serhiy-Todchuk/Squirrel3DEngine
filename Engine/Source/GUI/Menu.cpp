// Menu.cpp: implementation of the Menu class.
//
//////////////////////////////////////////////////////////////////////

#include "Menu.h"
#include "MenuItem.h"
#include "Cursor.h"
#include <Render/IRender.h>

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::Menu, GUIMenu);

using namespace Reflection;

const char_t * Menu::CLICKED_ACTION	= "clicked";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Menu::Menu():
	mSubMenu(NULL), mRootMenu(NULL)
{
	SQREFL_SET_CLASS(GUI::Menu);

	//init default cell creator
	mCellCreator = ObjectCreator::GetCreator("GUI::MenuItem");
	if(mCellCreator == NULL)
	{
		mCellCreator = ObjectCreatorFactory::Create<MenuItem>("GUI::MenuItem");
	}

	//TODO: make creation of scrolls at runtime
	deleteVerticalScroll();
	deleteHorizontalScroll();
}

Menu::~Menu()
{
	DELETE_PTR(mSubMenu);
}

void Menu::setVisible(bool visible)
{
	//if(v != getVisible())
	//	DELETE_PTR(mSubMenu);
	if(!visible && mSubMenu)
		mSubMenu->setVisible(false);
	Element::setVisible(visible);
}

bool Menu::isPointIn(tuple2i pt)
{
	if(mSubMenu)
	{
		if(mSubMenu->isPointIn(pt))
			return true;
	}
	return isIn(pt);
}

void Menu::setMenuLayout(tuple2i fulcrum, int width, bool bindToTheLeft)
{
	DELETE_PTR(mSubMenu);

	size_t mRowsNum = mContentSource->getRowsNum();
	size_t contentHeight = mRowsNum * mCellHeight;
	size_t height = contentHeight + mMargin.y + mMargin.w;

	tuple2i screen = Squirrel::Render::IRender::GetActive()->getWindow()->getSize();
	tuple2i size(width, height);

	tuple2i pos(fulcrum);//default pos directs menu body to the bottom-right

	int bottomMargin	= screen.y - (fulcrum.y + size.y);
	int topMargin		= fulcrum.y - size.y;

	//check vertical axis

	if(bottomMargin < 0)//bottom is bad
	{
		if(topMargin < 0)//top is also bad
		{
			if( topMargin > bottomMargin )//top is still better
			{
				size.y += topMargin;//decrease size from top side
				pos.y = fulcrum.y - size.y;
			}
			else//leave on bottom direction
			{
				size.y += bottomMargin;//decrease size from bottom side
			}
		}
		else//top is good
		{
			pos.y = fulcrum.y - size.y;
		}
	}

	//check horizontal axis

	if(!bindToTheLeft)
	{
		int rightMargin	= screen.x - (fulcrum.x + size.x);
		int leftMargin	= fulcrum.x - size.x;

		if(rightMargin < 0)//right is bad
		{
			if(leftMargin < 0)//left is also bad
			{
				if( leftMargin > rightMargin )//left is still better
				{
					size.x += leftMargin;//decrease size from left side
					pos.x = fulcrum.y - size.y;
				}
				else//leave on right direction
				{
					size.x += rightMargin;//decrease size from right side
				}
			}
			else//left is good
			{
				pos.x = fulcrum.x - size.x;
			}
		}
	}

	//set values
	setPos(pos);
	if(size != getSize())
	{
		setSize(size);
	}
}

void Menu::drawMenu()
{
	List::draw();

	if(mSubMenu && mSubMenu->getVisible())
		mSubMenu->drawMenu();
}

Element * Menu::recieveEvent(EventType e, int value)
{
	Element * elem = NULL;

	if(mSubMenu && mSubMenu->getVisible())
	{
		elem = mSubMenu->recieveEvent(e, value);
	}

	return elem != NULL ? elem : List::recieveEvent(e,value);
}

void Menu::showSubMenu(MenuContentSource * content, Element * pillar)
{
	if(mSubMenu == NULL)
	{
		mSubMenu = new Menu();
		mSubMenu->mRootMenu = mRootMenu == NULL ? this : mRootMenu;
	}

	if(mSubMenu->getContentSource() != content)
	{
		mSubMenu->setContentSource(content);
	}

	tuple2i fulcrum = getGlobalPos() + pillar->getPos() + tuple2i(getSize().x, 0);
	mSubMenu->setMenuLayout(fulcrum, getSize().x, true);

	mSubMenu->setVisible(true);
}

void Menu::processEventForCell(EventType e, int value, Element * cell)
{
	if(e == mouseMove)
	{
		if(cell->isIn(Cursor::Instance().getPos()))
		{
			select( cell->getId() );
			processAction(SELECTION_CHANGED_ACTION, this);
			
			MenuItem * item = static_cast<MenuItem *>(cell);
			if(item->mDesc)
			{
				if(item->mDesc->mSubmenu)
				{
					showSubMenu( item->mDesc->mSubmenu, cell );
				}
				else
				{
					//DELETE_PTR(mSubMenu);
					if(mSubMenu)
						mSubMenu->setVisible(false);
				}
			}
		}
	}
	else if(e == leftUp || e == rightUp)
	{
		processAction(CLICKED_ACTION, this);
		if(mRootMenu)
			mRootMenu->setVisible(false);
		else
			setVisible(false);
	}
}

}//namespace GUI { 
}//namespace Squirrel {