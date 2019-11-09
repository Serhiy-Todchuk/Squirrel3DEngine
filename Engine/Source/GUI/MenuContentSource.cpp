// MenuContentSource.cpp: implementation of the MenuContentSource class.
//
//////////////////////////////////////////////////////////////////////

#include "MenuContentSource.h"
#include "MenuItem.h"

namespace Squirrel {
namespace GUI { 

SQREFL_REGISTER_CLASS_SEED(GUI::MenuItemDesc, GUIMenuItemDesc);
SQREFL_REGISTER_CLASS_SEED(GUI::MenuContentSource, GUIMenuContentSource);

using namespace Reflection;

MenuItemDesc::MenuItemDesc():
	mSubmenu(NULL), mHandler(NULL)
{
	SQREFL_SET_CLASS(GUI::MenuItemDesc);

	wrapAtomicField("Caption",	&mCaption);
	wrapAtomicField("IconName",	&mIconName);
	//addField(		"Submenu",	 mSubmenu)->memoryMaster = false;
}

MenuItemDesc::~MenuItemDesc()
{
	DELETE_PTR(mSubmenu);
	DELETE_PTR(mHandler);
}

void MenuItemDesc::serialize(Reflection::Serializer * serializer)
{
	addField("Submenu",	mSubmenu)->memoryMaster = false;
	Reflection::Object::serialize(serializer);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MenuContentSource::MenuContentSource()
{
	SQREFL_SET_CLASS(GUI::MenuContentSource);

	wrapCollectionField<ITEM_DESC_LIST, MenuItemDesc>("Items", &mItems);
}

MenuContentSource::~MenuContentSource()
{
	for(ITEM_DESC_LIST::iterator it = mItems.begin(); it != mItems.end(); ++it)
	{
		DELETE_PTR((*it));
	}
}

MenuItemDesc& MenuContentSource::addItem(const std::string& caption)
{
	MenuItemDesc * itemDesc = new MenuItemDesc();
	itemDesc->mCaption = caption;
	mItems.push_back(itemDesc);
	return *itemDesc;
}

size_t MenuContentSource::getRowsNum()
{
	return mItems.size();
}

void MenuContentSource::setupCell(Element * cell, int row)
{
	MenuItem * itemControl = static_cast<MenuItem *>(cell);

	ASSERT(cell != NULL);
	ASSERT(row < mItems.size());

	ITEM_DESC_LIST::iterator it = mItems.begin();
	std::advance(it, row);
	MenuItemDesc * itemDesc = (*it);

	itemControl->mDesc = itemDesc;
	itemControl->setText(itemDesc->mCaption);
	itemControl->setName(itemDesc->mCaption);
	itemControl->setHasSubMenu(itemDesc->mSubmenu != NULL);

	itemControl->clearActions();
	if(itemDesc->mSubmenu == NULL && itemDesc->mHandler != NULL)
		itemControl->addAction(MenuItem::EXPOSE_ACTION, itemDesc->mHandler->clone());
}

}//namespace GUI { 
}//namespace Squirrel {