#pragma once

#include <Resource/ResourceManager.h>
#include <Resource/Skin.h>
#include "VisualSceneImporter.h"

namespace Squirrel {
namespace DAEImport { 

struct SkinController
{
	UniqueId				id;
	Resource::Skin *		skin;
	BufferArray<Math::mat4>	invBindMatrices;
	UniqueId				targetId;//mesh id
};

class SQDAEIMPORTER_API ControllerImporter: public BaseImporter
{
public:

	typedef std::map<UniqueId, SkinController *>	SKIN_CONTROLLER_MAP;


private:

	SKIN_CONTROLLER_MAP		mSkinControllers;

public:

	ControllerImporter(void);
	virtual ~ControllerImporter(void);

	virtual void importObject(pugi::xml_node node);

	SkinController * getSkinController(const UniqueId& skinControllerId);
};

}//namespace DAEImport { 
}//namespace Squirrel {