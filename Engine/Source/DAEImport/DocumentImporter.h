#pragma once

//Squirrel includes
#include <Resource/ModelImporter.h>

//Local includes
#include "MeshImporter.h"
#include "VisualSceneImporter.h"
#include "MaterialImporter.h"
#include "AnimationImporter.h"
#include "ControllerImporter.h"
#include "macros.h"

//other includes
#include <list>
#include <string>
#include <map>

namespace Squirrel {
namespace DAEImport { 

class SQDAEIMPORTER_API DocumentImporter : 
	public BaseImporter, public Resource::IModelImporter
{
private:

	virtual void importObject(pugi::xml_node node) {};

	bool parseXMLNode(pugi::xml_node node);
	void load(pugi::xml_document * doc, const std::string& modelName);

private: 

	typedef std::string String;

	Resource::Model *		mModel;

	MeshImporter			* mMeshImporter;
	VisualSceneImporter		* mVisualSceneImporter;
	MaterialImporter		* mMaterialImporter;
	AnimationImporter		* mAnimImporter;
	ControllerImporter		* mCtrlImporter;

	pugi::xml_node mCurrentLibraryNode;

	std::map<std::string, BaseImporter *>	mImporters;

	std::map<UniqueId, _ID>		mImportedSkinsIds;

	std::map<Resource::VertexBuffer *, SkinController *>	mSkinnedVBs;

	String mImportingStage;

public:
	DocumentImporter(Render::IRender * render, Resource::TextureStorage * texStorage, Resource::MaterialLibrary * matLib);
	virtual ~DocumentImporter(void);

	virtual bool load(const char_t * importFileName);
	virtual bool load(Data * data);

	virtual Resource::Model * getModel() {return mModel;}

	virtual const char_t * getImportingExtension() const {
		return "dae";
	}

	const char_t * getImportingStageDesc() const;

private:

	 void setImportStage(const char_t * str);

	void finish();

	Resource::AnimationTrack * bakeNodeAnims(Node * node, bool convertMatrices, float timeStep, size_t& aproxNumberOfFrames);

	void applyBindTransform(Resource::VertexBuffer * vb, SkinController * skinController);
	void importNode(Node * node, Resource::Model::Node * parentLink);
	SkinController * importNodeSkin(Node * node, Resource::Model::Node * modelNode);
};

}//namespace DAEImport { 
}//namespace Squirrel {