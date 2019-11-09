#include "ResourceManager.h"
#include "Common/Settings.h"
#include "FileSystem/Path.h"

namespace Squirrel {

namespace Resource {

ResourceManager::ResourceManager():
	mMaterialLibrary(new MaterialLibrary),
	mTextureStorage(new TextureStorage),
	mModelStorage(new ModelStorage),
	mProgramStorage(new ProgramStorage),
	mSoundStorage(new SoundStorage)
{
	
}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::bind()
{
	mMaterialLibrary	-> setAsActive();
	mTextureStorage		-> setAsActive();
	mModelStorage		-> setAsActive();
	mProgramStorage		-> setAsActive();
	mSoundStorage		-> setAsActive();
}

void ResourceManager::initContentSource(const std::string& section)
{
	std::string texuresPath = Settings::Default()->getString(section.c_str(), "Textures storage", "Textures");
	texuresPath = FileSystem::Path::GetAbsPath(texuresPath);
	mTextureStorage->initContentSource( texuresPath );

	std::string modelsPath = Settings::Default()->getString(section.c_str(), "Models storage", "Models");
	modelsPath = FileSystem::Path::GetAbsPath(modelsPath);
	mModelStorage->initContentSource( modelsPath );

	std::string programsPath = Settings::Default()->getString(section.c_str(), "Programs storage", "Shaders");
	programsPath = FileSystem::Path::GetAbsPath(programsPath);
	mProgramStorage->initContentSource( programsPath );

	std::string soundsPath = Settings::Default()->getString(section.c_str(), "Sounds storage", "Sounds");
	soundsPath = FileSystem::Path::GetAbsPath(soundsPath);
	mSoundStorage->initContentSource( soundsPath );
}

}//namespace Resource { 

}//namespace Squirrel {