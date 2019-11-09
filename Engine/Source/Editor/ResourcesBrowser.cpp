#include "ResourcesBrowser.h"
#include <FileSystem/Path.h>
#include <GUI/Manager.h>
#include <GUI/Button.h>
#include <GUI/Label.h>
#include <GUI/Edit.h>
#include <GUI/Switch.h>
#include <GUI/Window.h>

namespace Squirrel {
namespace Editor {

const char_t * PREVIOUS_FOLDER_NAME	= "..";

ResourcesBrowser::ResourcesBrowser()
{
	mResourceOpenDelegate = NULL;

	mPanel = new GUI::Window();
	mPanel->setName("resourcesPanel");
	mPanel->setText("Models Browser");
	mPanel->setPosSep(5, 170);
	mPanel->setSizeSep(246, 200);
	GUI::Manager::Instance().getMainPanel().addPanel( mPanel );

	mList = mPanel->add<GUI::List>("resourcesBrowser", 0, 2, 240, 180);
	mList->setContentSource( this );
	mList->setActionDelegate( this );

	mList->setVerticalSizing(	GUI::Element::bindFlexible );
	mList->setHorizontalSizing(	GUI::Element::bindFlexible );
	mList->setBindingBottom(	GUI::Element::bindStrictly );
	mList->setBindingLeft(		GUI::Element::bindStrictly );
	mList->setBindingTop(		GUI::Element::bindStrictly );
	mList->setBindingRight(		GUI::Element::bindStrictly );

	//hide
	mPanel->setVisible(false);
}

ResourcesBrowser::~ResourcesBrowser()
{
	GUI::Manager::Instance().getMainPanel().delPanel( mPanel );
	DELETE_PTR(mPanel);
}

void ResourcesBrowser::show()
{
	checkoutFiles();
	//mList->update();
	mPanel->setVisible(true);
}

void ResourcesBrowser::close()
{
	mPanel->setVisible(false);
}

void ResourcesBrowser::checkoutFiles()
{
	std::string location = "";
	for(	std::list<std::string>::iterator it = mLocation.begin();
			it != mLocation.end(); ++it	)
	{
		location = FileSystem::Path::Combine(location, (*it));
	}

	mFiles.clear();

	const std::list<FileSystem::FileInfo>& filesList = mResourcesContainer->getContent(location.c_str());
	std::list<FileSystem::FileInfo>::const_iterator itFile;
	for(itFile = filesList.begin(); itFile != filesList.end(); ++itFile)
	{
		//skip previous folder if we r in the root of resource container
		if(mLocation.size() == 0)
		{
			if((*itFile).name == PREVIOUS_FOLDER_NAME) continue;
		}
		//skip disallowed file types
		if(mAllowedExtensions.size() != 0 && !(*itFile).isFolder)
		{
			if(!isExtensionAllowed((*itFile).extension)) continue;
		}
		//skip hidden files
		if((*itFile).isHidden) continue;

		//add new file
		mFiles.push_back((*itFile));
	}
}

size_t ResourcesBrowser::getRowsNum()
{
	return mFiles.size();
}

void ResourcesBrowser::setupCell(GUI::Element * cell, int row)
{
	std::string displayName = "";

	FileSystem::FileInfo &fileInfo = mFiles[row];
	if(fileInfo.isFolder)
	{
		displayName = "[F] ";
	}
	else
	{
		displayName = "     ";
	}

	displayName += fileInfo.name;
	cell->setText((char *)displayName.c_str());
}

bool ResourcesBrowser::processAction(const std::string& action, GUI::Element * sender)
{
	if(sender == NULL) return false;
	if(sender != mList) return false;

	if(action == GUI::List::DOUBLECLICKED_ACTION)
	{
		int selectedFileIndex = mList->getSelected();
		if(selectedFileIndex < 0) return false;

		FileSystem::FileInfo& fileInfo = mFiles[selectedFileIndex];
		if(fileInfo.isFolder)
		{
			if(fileInfo.name == PREVIOUS_FOLDER_NAME)
			{
				mLocation.pop_back();
			}
			else
			{
				mLocation.push_back(fileInfo.name);
			}
			//mList->update();
			mList->select(0);
		}
		else
		{
			if(mResourceOpenDelegate != NULL)
			{
				mResourceOpenDelegate->resourceOpened(fileInfo);
			}
		}

		checkoutFiles();

		return true;
	}
	return false;
}

}//namespace Render { 
}//namespace Squirrel {

