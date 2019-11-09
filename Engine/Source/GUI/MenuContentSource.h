#pragma once

#include "List.h"

namespace Squirrel {
namespace GUI {

class MenuContentSource;

class MenuItemDesc : 
	public Reflection::Object
{
public:
	MenuItemDesc();
	virtual ~MenuItemDesc();

	std::string mIconName;
	std::string mCaption;
	std::string mHotkey;
	
	ActionHandler * mHandler;

	MenuContentSource * mSubmenu;

	virtual void serialize(Reflection::Serializer * serializer);
};

class SQGUI_API MenuContentSource : 
	public Reflection::Object, 
	public ListContentSource
{
public:
	MenuContentSource();
	virtual ~MenuContentSource();

	typedef std::list<MenuItemDesc *> ITEM_DESC_LIST;

	MenuItemDesc& addItem(const std::string& caption);

	//ListContentSource implementation
	virtual size_t		getRowsNum();
	virtual void		setupCell(Element * cell, int row);

private:

	ITEM_DESC_LIST mItems;


};

}//namespace GUI { 
}//namespace Squirrel {