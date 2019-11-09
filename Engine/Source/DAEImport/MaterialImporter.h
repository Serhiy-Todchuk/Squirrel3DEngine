#pragma once

#include "BaseImporter.h"

namespace Squirrel {
namespace DAEImport { 

class SQDAEIMPORTER_API MaterialImporter: public BaseImporter
{
private:

	struct MaterialLink
	{
		Resource::Material *	material;
		UniqueId				diffuseTexture;
		UniqueId				specularTexture;
		UniqueId				bumpTexture;
		UniqueId				heightTexture;
	};

    std::map<UniqueId, MaterialLink>		mEffectsList;
    std::map<UniqueId, String>				mImagesMap;
    std::map<UniqueId, UniqueId>			mMaterials2EffectsMap;

public:

	MaterialImporter(void);
	virtual ~MaterialImporter(void);

	virtual void importObject(pugi::xml_node node);

	bool getMaterialForId(	const UniqueId &fwMatId, 
							Resource::TextureStorage * texLib, 
							Resource::MaterialLibrary * matLib, 
							Resource::Model::MaterialLink& matLink);

private:

	void addMaterial	(pugi::xml_node node);
	void addImage		(pugi::xml_node node);
	void addEffect		(pugi::xml_node node);

	_ID getImage(UniqueId imageId, Resource::TextureStorage * texLib);
	String getImagePath(UniqueId imageId);
};

}//namespace DAEImport { 
}//namespace Squirrel {