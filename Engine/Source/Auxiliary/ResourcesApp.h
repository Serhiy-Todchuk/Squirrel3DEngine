// ResourcesApp.h: interface for the ResourcesApp class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Application.h"
#include <Resource/ResourceManager.h>

namespace Squirrel {

namespace Auxiliary {
	
class ResourcesApp:
	public Application
{
public:	
    /** Constructor */
	ResourcesApp();
    /** Destructor */
	virtual ~ResourcesApp();
    
    virtual bool init();

	virtual void onBecomeActive();

	Resource::ResourceManager * getResourceManager() { return mResourceManager; }
	
protected:

    virtual void initResources() {};
	
    /** Initialises resource storages. 
     @param rootPath Path to root folder where all resources are stored.
	 If it is NULL, the path will be read from Settings object obtained with getSettings method:
	 section - Path, parameter - Root Path.
     */
	void initResourceManagement(const char_t * rootPath = NULL);

protected:

	Resource::ResourceManager * mResourceManager;
};

}//namespace Squirrel {
}//namespace Auxiliary {
