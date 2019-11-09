#pragma once

#include <common/common.h>
#include <Resource/ModelImporter.h>
#include <Resource/TextureStorage.h>
#include <Common/DynamicLibrary.h>
#include <Common/Context.h>
#include <Common/Thread.h>
#include "macros.h"

namespace Squirrel {
namespace Editor { 

typedef Resource::IModelImporter * (*CreateImporterFuncPtr)(Render::IRender * render, Resource::TextureStorage * texStorage);
typedef void (*DestroyImporterFuncPtr)(Resource::IModelImporter * importer);

class SQEDITOR_API ModelImporter:
	Runnable
{
public:
	ModelImporter(const char_t * moduleName, Render::IRender * render, Resource::TextureStorage * texStorage);
	virtual ~ModelImporter();

	bool startImport(Data * data, Context * ctx);
	bool startImport(const std::string& fileName, Context * ctx);

	const char_t * getStageDesc();

	Resource::Model * getModel();

	bool isOk() const		{ return mIsOk; }
	bool isRunning() const	{ return mImportThread != NULL; }

private:

	bool initModule(const char_t * moduleName, Render::IRender * render, Resource::TextureStorage * texStorage);
	void closeModule();

	virtual void* run();

	DynamicLibrary mModelImporterLib;
	Thread * mImportThread;

	Resource::IModelImporter * mModelImporter;

	CreateImporterFuncPtr	CreateImporter;
	DestroyImporterFuncPtr	DestroyImporter;

	Context * mRenderingContext;

	Data * mData;
	std::string mFileName;

	bool mIsOk;
};

}//namespace Editor { 
}//namespace Squirrel {
