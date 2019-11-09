#pragma once

//Squirrel includes
#include <Resource/ModelImporter.h>

//Local includes
#include "macros.h"

//FBX SDK
#define FBXSDK_NEW_API
#include <fbxsdk.h>

//other includes
#include <list>
#include <string>
#include <map>

namespace Squirrel {
namespace FBXImport { 

class SQFBXIMPORTER_API DocumentImporter : 
	public Resource::IModelImporter
{
private: 

	typedef std::string String;

	Resource::Model *		mModel;

	String mImportingStage;

public:
	DocumentImporter(Render::IRender * render, Resource::TextureStorage * texStorage, Resource::MaterialLibrary * matLib);
	virtual ~DocumentImporter(void);

	virtual bool load(const char_t * importFileName);
	virtual bool load(Data * data);

	virtual Resource::Model * getModel() {return mModel;}

	virtual const char_t * getImportingExtension() const {
		return "fbx";
	}

	const char_t * getImportingStageDesc() const;

private:

	void setImportStage(const char_t * str);

	bool importScene(FbxScene * scene);
	void importNode(FbxNode * node, Resource::Model::Node * parentNode);

	Resource::Mesh * importMesh(const FbxMesh *pMesh, Resource::Model::Node * modelNode);
};

}//namespace FBXImport { 
}//namespace Squirrel {