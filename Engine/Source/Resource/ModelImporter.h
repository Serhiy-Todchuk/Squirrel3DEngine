#pragma once

#include <Render/IRender.h>
#include "TextureStorage.h"
#include "MaterialLibrary.h"
#include "Model.h"
#include "macros.h"

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API IModelImporter
{
protected:
	Render::IRender * mRender;
	Resource::TextureStorage * mTexturesStorage;
	Resource::MaterialLibrary * mMaterialLibrary;

public:
	IModelImporter(Render::IRender * render, Resource::TextureStorage * texStorage, Resource::MaterialLibrary * matLib);
	virtual ~IModelImporter();

	virtual bool load(const char_t * importFileName) = 0;
	virtual bool load(Data * data) = 0;

	virtual Resource::Model * getModel() = 0;

	virtual const char_t * getImportingExtension() const = 0;

	virtual const char_t * getImportingStageDesc() const = 0;
};


}//namespace Resource { 

}//namespace Squirrel {