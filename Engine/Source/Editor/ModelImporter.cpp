#include "Editor.h"

namespace Squirrel {
namespace Editor {

ModelImporter::ModelImporter(const char_t * moduleName, Render::IRender * render, Resource::TextureStorage * texStorage):
	mModelImporter(NULL), CreateImporter(NULL), DestroyImporter(NULL), mRenderingContext(NULL), mImportThread(NULL)
{
	mIsOk = initModule(moduleName, render, texStorage);
}

ModelImporter::~ModelImporter()
{
	closeModule();
}

bool ModelImporter::initModule(const char_t * moduleName, Render::IRender * render, Resource::TextureStorage * texStorage)
{
	closeModule();

	std::string fullModuleName = moduleName;
#if defined(_DEBUG) && defined(_WIN32)
	fullModuleName += "D";
#endif
	fullModuleName += ".";
	fullModuleName += DynamicLibrary::GetDLExtension();
	
	mModelImporterLib.open(fullModuleName.c_str());

	if(!mModelImporterLib.isOpened())
		return false;

	CreateImporter = (CreateImporterFuncPtr)mModelImporterLib.getSymbol("CreateImporter");
	DestroyImporter = (DestroyImporterFuncPtr)mModelImporterLib.getSymbol("DestroyImporter");

	if(!CreateImporter || !DestroyImporter)
		return false;

	mModelImporter = CreateImporter(render, texStorage);

	if(!mModelImporter)
	{
		closeModule();
		return false;
	}

	return true;
}

void ModelImporter::closeModule()
{
	if(!mModelImporterLib.isOpened())
		return;

	if(mModelImporter)
	{
		DestroyImporter(mModelImporter);
	}

	mModelImporterLib.close();
}


bool ModelImporter::startImport(Data * data, Context * ctx)
{
	if(mImportThread != NULL)
		return false;

	ASSERT(data != NULL);
	ASSERT(ctx != NULL);
	ASSERT(mModelImporter != NULL);

	mRenderingContext = ctx;

	mData = data;

	mImportThread = Thread::Create(this);
	mImportThread->start();

	return true;
}

bool ModelImporter::startImport(const std::string& fname, Context * ctx)
{
	if(mImportThread != NULL)
		return false;
	
	ASSERT(ctx != NULL);
	ASSERT(mModelImporter != NULL);

	mRenderingContext = ctx;

	mData = NULL;
	mFileName = fname;

	mImportThread = Thread::Create(this);
	mImportThread->start();
	
	return true;
}

const char_t * ModelImporter::getStageDesc()
{
	ASSERT(mModelImporter);

	return mModelImporter->getImportingStageDesc();
}

Resource::Model * ModelImporter::getModel()
{
	if(mImportThread && mImportThread->isFinished())
	{
		DELETE_PTR(mImportThread);
		return mModelImporter->getModel();
	}

	return NULL;
}

void* ModelImporter::run()
{
	ASSERT(mRenderingContext);
	ASSERT(mModelImporter);

	mRenderingContext->activate();

	if(mData != NULL)
		mModelImporter->load(mData);
	else
		mModelImporter->load(mFileName.c_str());

	mRenderingContext->deactivate();

	return NULL;
}

}//namespace Render { 
}//namespace Squirrel {

