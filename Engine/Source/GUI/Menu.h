#pragma once

#include "List.h"
#include "MenuContentSource.h"

namespace Squirrel {
namespace GUI {

class SQGUI_API Menu  : 
	public List
{
public:
	static const char_t * CLICKED_ACTION;

public:
	Menu();
	virtual ~Menu();

	void setMenuLayout(tuple2i fulcrum, int width, bool bindToTheLeft);

	bool isPointIn(tuple2i pt);

	virtual void drawMenu();
	virtual void draw() {};

	virtual void setVisible(bool v);

	virtual Element * recieveEvent(EventType e, int value);

protected:

	virtual void processEventForCell(EventType e, int value, Element * cell);

private:

	void setCellCreator(Reflection::ObjectCreator * cellCreator) { List::setCellCreator(cellCreator); }
	void showSubMenu(MenuContentSource * content, Element * pillar);

private:
	Menu * mSubMenu;
	Menu * mRootMenu;
};



}//namespace GUI { 
}//namespace Squirrel {