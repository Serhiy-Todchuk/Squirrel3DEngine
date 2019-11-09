#include "MaterialImporter.h"
#include <Common/Settings.h>
#include "Arrays.h"

namespace Squirrel {
namespace DAEImport { 

#define	EFFECT_TAG			"effect"
#define	MATERIAL_TAG		"material"
#define IMAGE_TAG			"image"
#define INIT_FROM_TAG		"init_from"
#define INSTANCE_EFFECT_TAG	"instance_effect"
#define PROFILE_COMMON_TAG	"profile_COMMON"
#define NEWPARAM_TAG		"newparam"
#define SAMPLER2D_TAG		"sampler2D"
#define SURFACE_TAG			"surface"
#define TECHNIQUE_TAG		"technique"
#define EXTRA_TAG			"extra"

#define BLINN_TAG			"blinn"
#define LAMBERT_TAG			"lambert"
#define CONSTANT_TAG		"constant"
#define PHONG_TAG			"phong"

#define EMISSION_TAG		"emission"
#define AMBIENT_TAG			"ambient"
#define SPECULAR_TAG		"specular"
#define SHININESS_TAG		"shininess"
#define DIFFUSE_TAG			"diffuse"
#define REFLECTIVE_TAG		"reflective"
#define TRANSPARENT_TAG		"transparent"
#define TRANSPARENCY_TAG	"transparency"

#define COLOR_TAG			"color"
#define TEXTURE_TAG			"texture"
#define FLOAT_TAG			"float"

	
MaterialImporter::MaterialImporter(void)
{
}

MaterialImporter::~MaterialImporter(void)
{
}

void MaterialImporter::importObject(pugi::xml_node node)
{
	if(String(node.name()) == EFFECT_TAG)
	{
		addEffect(node);
	}
	else if(String(node.name()) == MATERIAL_TAG)
	{
		addMaterial(node);
	}
	else if(String(node.name()) == IMAGE_TAG)
	{
		addImage(node);
	}
}

void MaterialImporter::addMaterial(pugi::xml_node node)
{
	UniqueId matId = node.attribute(ID_ATTR.c_str()).value();

	pugi::xml_node instEffectNode = node.child(INSTANCE_EFFECT_TAG);

	UniqueId effectUrl = instEffectNode.attribute(URL_ATTR.c_str()).value();
	UniqueId effectId = effectUrl.substr(1);//skip first character as it is '#'

	mMaterials2EffectsMap[matId] = effectId;
}

void MaterialImporter::addImage(pugi::xml_node node)
{
	UniqueId imgId = node.attribute(ID_ATTR.c_str()).value();

	pugi::xml_node imgSrcNode = node.child(INIT_FROM_TAG);

	String imgSrc = imgSrcNode.child_value();

	mImagesMap [imgId] = imgSrc;
}

void MaterialImporter::addEffect(pugi::xml_node node)
{
	UniqueId effectId = node.attribute(ID_ATTR.c_str()).value();

	pugi::xml_node profileNode = node.child(PROFILE_COMMON_TAG);

	//parse params

	std::map<UniqueId, UniqueId> samplerParams;
	std::map<UniqueId, UniqueId> surfaceParams;

	for(pugi::xml_node paramNode = profileNode.child(NEWPARAM_TAG); paramNode; paramNode = paramNode.next_sibling(NEWPARAM_TAG))
	{
		UniqueId paramId = paramNode.attribute(SUB_ID_ATTR.c_str()).value();

		pugi::xml_node sampler2DNode = paramNode.child(SAMPLER2D_TAG);
		if(sampler2DNode)
		{
			pugi::xml_node samplerSourceNode = sampler2DNode.child(SOURCE_TAG.c_str());

			UniqueId samplerSource = samplerSourceNode.child_value();

			samplerParams[paramId] = samplerSource;

			continue;
		}

		pugi::xml_node surfaceNode = paramNode.child(SURFACE_TAG);
		if(surfaceNode)
		{
			pugi::xml_node initFromNode = surfaceNode.child(INIT_FROM_TAG);

			UniqueId initFrom = initFromNode.child_value();

			surfaceParams[paramId] = initFrom;

			continue;
		}
	}

	//extract samplerId->imageId map

	std::map<UniqueId, UniqueId> imageIdsMap;

	for(std::map<UniqueId, UniqueId>::iterator itSampler = samplerParams.begin(); itSampler != samplerParams.end(); ++itSampler)
	{
		UniqueId samplerId = itSampler->first;

		UniqueId surfaceId = itSampler->second;

		std::map<UniqueId, UniqueId>::iterator itSurface = surfaceParams.find(surfaceId);

		if(itSurface != surfaceParams.end())
		{
			UniqueId imageId = itSurface->second;

			imageIdsMap[samplerId] = imageId;
		}
	}

	//create Material

	MaterialLink outLink;
	RenderData::Material * outMat = new RenderData::Material();
	outLink.material = outMat;

	//parse technique

	pugi::xml_node techNode = profileNode.child(TECHNIQUE_TAG);

	pugi::xml_node shadeNode = techNode.child(BLINN_TAG);
	if(!shadeNode) shadeNode = techNode.child(LAMBERT_TAG);
	if(!shadeNode) shadeNode = techNode.child(CONSTANT_TAG);
	if(!shadeNode) shadeNode = techNode.child(PHONG_TAG);

	ASSERT(!shadeNode.empty());

	outMat->mLighting = ( shadeNode.name() );

	pugi::xml_node colorOrTextureNode;
	pugi::xml_node floatHolderNode;

	//emission
	colorOrTextureNode = shadeNode.child(EMISSION_TAG);
	if(colorOrTextureNode)
	{
		pugi::xml_node colorNode = colorOrTextureNode.child(COLOR_TAG);
		if(colorNode)
		{
			FloatArray color(4, colorNode.child_value());
			outMat->mEmission = (Math::vec4(color.getArray()));
		}
	}

	//ambient
	colorOrTextureNode = shadeNode.child(AMBIENT_TAG);
	if(colorOrTextureNode)
	{
		pugi::xml_node colorNode = colorOrTextureNode.child(COLOR_TAG);
		if(colorNode)
		{
			FloatArray color(4, colorNode.child_value());
			outMat->mAmbient = (Math::vec4(color.getArray()));
		}
	}
	
	//reflective
	colorOrTextureNode = shadeNode.child(REFLECTIVE_TAG);
	if(colorOrTextureNode)
	{
		pugi::xml_node colorNode = colorOrTextureNode.child(COLOR_TAG);
		if(colorNode)
		{
			FloatArray color(4, colorNode.child_value());
			outMat->mReflect = (Math::vec4(color.getArray()));
		}
	}

	//diffuse
	colorOrTextureNode = shadeNode.child(DIFFUSE_TAG);
	if(colorOrTextureNode)
	{
		pugi::xml_node colorNode = colorOrTextureNode.child(COLOR_TAG);
		pugi::xml_node textureNode = colorOrTextureNode.child(TEXTURE_TAG);
		if(colorNode)
		{
			FloatArray color(4, colorNode.child_value());
			outMat->mDiffuse = (Math::vec4(color.getArray()));
		}
		else if(textureNode)
		{
			UniqueId samplerId = textureNode.attribute(TEXTURE_TAG).value();

			std::map<UniqueId, UniqueId>::iterator itImage = imageIdsMap.find(samplerId);

			if(itImage != imageIdsMap.end())
			{
				UniqueId imageId = itImage->second;
				outLink.diffuseTexture = imageId;
			}
			else
			{
				outLink.diffuseTexture = samplerId;
			}
		}
	}

	//specular
	colorOrTextureNode = shadeNode.child(SPECULAR_TAG);
	if(colorOrTextureNode)
	{
		pugi::xml_node colorNode = colorOrTextureNode.child(COLOR_TAG);
		pugi::xml_node textureNode = colorOrTextureNode.child(TEXTURE_TAG);
		if(colorNode)
		{
			FloatArray color(4, colorNode.child_value());
			outMat->mSpecular = (Math::vec4(color.getArray()));
		}
		else if(textureNode)
		{
			UniqueId samplerId = textureNode.attribute(TEXTURE_TAG).value();

			std::map<UniqueId, UniqueId>::iterator itImage = imageIdsMap.find(samplerId);

			if(itImage != imageIdsMap.end())
			{
				UniqueId imageId = itImage->second;
				outLink.specularTexture = imageId;
			}
			else
			{
				outLink.specularTexture = samplerId;
			}
		}
	}

	//shininess
	floatHolderNode = shadeNode.child(SHININESS_TAG);
	if(floatHolderNode)
	{
		pugi::xml_node floatNode = floatHolderNode.child(FLOAT_TAG);
		if(floatNode)
		{
			float value = static_cast<float>(atof(floatNode.child_value()));
			outMat->mShininess = (value);
		}
	}

	//transparency
	floatHolderNode = shadeNode.child(TRANSPARENCY_TAG);
	if(floatHolderNode)
	{
		pugi::xml_node floatNode = floatHolderNode.child(FLOAT_TAG);
		if(floatNode)
		{
			float value = static_cast<float>(atof(floatNode.child_value()));
			outMat->mOpacity = (1.0f - value);
		}
	}

	//extract openCollada bump map extra

	pugi::xml_node extraNode = techNode.child(EXTRA_TAG);
	if(extraNode)
	{
		pugi::xml_node extraTechNode = extraNode.child(TECHNIQUE_TAG);
		if(extraTechNode)
		{
			pugi::xml_node bumpNode = extraTechNode.child("bump");
			if(bumpNode)
			{
				pugi::xml_node textureNode = bumpNode.child(TEXTURE_TAG);
				if(textureNode)
				{
					UniqueId samplerId = textureNode.attribute(TEXTURE_TAG).value();

					std::map<UniqueId, UniqueId>::iterator itImage = imageIdsMap.find(samplerId);

					if(itImage != imageIdsMap.end())
					{
						UniqueId imageId = itImage->second;
						outLink.bumpTexture = imageId;
					}
					else
					{
						outLink.bumpTexture = samplerId;
					}
				}
			}
		}
	}

	mEffectsList[effectId] = outLink;
}

String MaterialImporter::getImagePath(UniqueId imageId)
{
	if(imageId.length() > 0)
	{
		std::map<UniqueId, String>::const_iterator itImg = mImagesMap.find(imageId);
		if(itImg != mImagesMap.end())
		{
			String imgPath = itImg->second;//imgUri->getPath();
#ifdef _WIN32//remove '\' char before disk name
			int dskPos = imgPath.find_first_of(':');
			if(dskPos > 1)
			{
				--dskPos;
				imgPath = imgPath.substr(dskPos, imgPath.length() - dskPos);
			}
#endif
			return imgPath;
		}
	}

	return String("");
}

_ID MaterialImporter::getImage(UniqueId imageId, Resource::TextureStorage * texLib)
{
	String imagePath = getImagePath(imageId);
	if(imagePath.length() > 0)
	{
		return texLib->addRetId(imagePath);
	}

	return _INVALID_ID;
}

bool MaterialImporter::getMaterialForId(	const UniqueId &matId, 
												Resource::TextureStorage * texLib, 
												Resource::MaterialLibrary * matLib, 
												Resource::Model::MaterialLink& matLink)
{
	std::map<UniqueId, UniqueId>::const_iterator itMat2Eff = mMaterials2EffectsMap.find(matId);

	//found nothing?
	if(itMat2Eff == mMaterials2EffectsMap.end()) 
		return false;

	UniqueId effectId = itMat2Eff->second;

	//get material effect
	std::map<UniqueId, MaterialLink>::const_iterator itEff = mEffectsList.find(effectId);

	//found nothing?
	if(itEff == mEffectsList.end()) 
		return false;

	MaterialLink rawLink = itEff->second;

	rawLink.material->setName(effectId);//TODO: read material name and put here

	matLink.idMaterial = matLib->add(rawLink.material);
	matLink.mMaterial = rawLink.material;

	//Load textures

	//diffuse
	if(rawLink.diffuseTexture.length() > 0)
	{
		matLink.idTexDiffuse = getImage(rawLink.diffuseTexture, texLib);
	}

	//bump
	if(rawLink.bumpTexture.length() > 0)
	{
		Resource::Texture * tex = texLib->loadNormalMap(getImagePath(rawLink.bumpTexture), true);
		if(tex)
			matLink.idTexHeightBump = tex->getID();
	}
	if(matLink.idTexHeightBump < 0)
	{
		bool forceNHMGen = Settings::Default()->getInt("Resources", "ForceNHMGen", 1) != 0;
		if(forceNHMGen)
		{
			matLink.generateBumpHeightMap(getImagePath(rawLink.diffuseTexture));
		}
	}

	//specular
	if(rawLink.specularTexture.length() > 0)
	{
		matLink.idTexSpecular = getImage(rawLink.specularTexture, texLib);
	}

	return true;
}

}//namespace DAEImport { 
}//namespace Squirrel {