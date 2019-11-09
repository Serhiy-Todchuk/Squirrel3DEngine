#pragma once

#include <common/common.h>
#include <GUI/Panel.h>
#include <GUI/List.h>
#include <FileSystem/FileStorage.h>
#include <set>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

class SQEDITOR_API ResourceOpenDelegate
{
public:
	ResourceOpenDelegate() {}
	virtual ~ResourceOpenDelegate() {}

	virtual void resourceOpened(const FileSystem::FileInfo& resourceFile) = 0;
};

class SQEDITOR_API ResourcesBrowser: 
	public GUI::ListContentSource,
	public GUI::ActionDelegate
{
public://ctor/dtor
	ResourcesBrowser();
	~ResourcesBrowser();

public://methods

	//mResourceOpenDelegate setter
	void setResourceOpenDelegate(ResourceOpenDelegate * delg) { mResourceOpenDelegate = delg; }

	//resourceContainer setter
	void setResourceContainer(FileSystem::FileStorage * cont) { mResourcesContainer = cont; }

	//main control
	void show();
	void close();

	//extensions management
	inline void clearAllowedExtensions()	
		{ mAllowedExtensions.clear(); }

	inline void addAllowedExtension(const std::string& ext)	
		{ mAllowedExtensions.insert(ext); }

	bool isExtensionAllowed(const std::string& ext) const 
		{ return mAllowedExtensions.find(ext) != mAllowedExtensions.end(); }

private://members

	//UI content
	GUI::Panel * mPanel;
	GUI::List * mList;

	//callback
	ResourceOpenDelegate * mResourceOpenDelegate;

	//file system
	std::list<std::string> mLocation;
	std::vector<FileSystem::FileInfo> mFiles;
	FileSystem::FileStorage * mResourcesContainer;
	std::set<std::string> mAllowedExtensions;

private://methods

	//ListContentSource implementation
	size_t		getRowsNum();
	void		setupCell(GUI::Element * cell, int row);

	//ActionDelegate implementation
	virtual bool processAction(const std::string& action, GUI::Element * sender);

	//internal utils
	void checkoutFiles();
};

}//namespace Editor { 
}//namespace Squirrel {
