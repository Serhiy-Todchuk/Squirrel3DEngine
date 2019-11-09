// ResourcesApp.cpp: implementation of the ResourcesApp class.
//
//////////////////////////////////////////////////////////////////////

#include "ResourcesApp.h"
#include <FileSystem/Path.h>

namespace Squirrel {
namespace Auxiliary {
	
ResourcesApp::ResourcesApp(): mResourceManager(NULL)
{
}

ResourcesApp::~ResourcesApp()
{
}

bool ResourcesApp::init()
{
    bool success = Application::init();
	
	if(!success)
		return success;
	
	initResourceManagement();
	
	initResources();
	
	return success;
}

void ResourcesApp::onBecomeActive()
{
	if(mResourceManager == NULL)
		return;

	if(mResourceManager->getProgramStorage() == NULL)
		return;

	if(Render::IRender::GetActive())
		Render::IRender::GetActive()->flush(true);

	mResourceManager->getProgramStorage()->checkModifications();
}

void ResourcesApp::initResourceManagement(const char_t * rootPath)
{
	std::string rootPathStr;
	
	if(rootPath == NULL)
	{
		rootPathStr = getSettings()->getString("Path", "Root Path", "..");
	}
	else
	{
		rootPathStr = rootPath;
	}
	
	if(Settings::Default() == NULL)
		getSettings()->setAsDefault();
	
	FileSystem::Path::InitRootPath(rootPathStr);
	Log::Instance().report("Root path", FileSystem::Path::GetAbsPath("").c_str(), Log::sevInformation);
	
	//init resource system
	mResourceManager = new Resource::ResourceManager();
	mResourceManager->initContentSource(RESOURCES_SETTINGS_SECTION);
	mResourceManager->bind();
	
	mDestructionPool->addObject(mResourceManager);
}

}//namespace Squirrel {
}//namespace Auxiliary {
