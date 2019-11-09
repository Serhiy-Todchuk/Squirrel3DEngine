#pragma once

#include "Element.h"
#include "MenuContentSource.h"

namespace Squirrel {
namespace GUI {

class SQGUI_API MenuItem  : public Element
{
public:

	static const char_t * EXPOSE_ACTION;

	enum Column
	{
		columnIcon = 0,
		columnCaption,
		columnHotkey,
		columnSubmenu,
		columnsNum
	};

private:

	float mColumnWidth[columnsNum];

	bool mSubMenu;

	std::string mHotkey;

public:
	MenuItem();
	virtual ~MenuItem();

	MenuItemDesc * mDesc;

	virtual void draw();

	virtual Element * onLU();
	virtual Element * onRU();

	virtual Element * onLD()		{return this;}
	virtual Element * onRD()		{return this;}
	virtual Element * onMM()		{return this;}

	bool hasSubMenu() { return mSubMenu; }
	void setHasSubMenu(bool has) { mSubMenu = has; }

	float getColumnWidth(Column column) { return mColumnWidth[column]; }
	void setColumnWidth(float width, Column column) { mColumnWidth[column] = width; }
};



}//namespace GUI { 
}//namespace Squirrel {