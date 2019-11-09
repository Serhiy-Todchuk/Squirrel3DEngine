#pragma once

#include "ResourceStorage.h"
#include "ImageLoader.h"
#include "Texture.h"

namespace Squirrel {
namespace Resource { 

#ifdef _WIN32
SQRESOURCE_TEMPLATE template class SQRESOURCE_API ResourceStorage<Texture>;
#endif
	
//TODO: create base class Library (analog of singleton template impl)
class SQRESOURCE_API TextureStorage: 
	public ResourceStorage<Texture>
{
	static TextureStorage * sActiveLibrary;

public:
	
	const static int DEFAULT_ATLAS_SIZE = 2048;
	
public:
	TextureStorage();
	virtual ~TextureStorage();

	void deleteSourceImages(bool keepChanged = true);

	Texture * loadTexture(const std::string& fileName, bool dontBuildMipmaps = false, bool dontCompress = false);

	Texture * makeCubemap(std::string fileNames[Render::ITexture::cmfNum]);
	Texture * makeGridAtlas(const std::list<std::string>& fileNames,
							tuple2i atlasSize = tuple2i(DEFAULT_ATLAS_SIZE, DEFAULT_ATLAS_SIZE),
							tuple2i biasBefore = tuple2i(0, 0),
							tuple2i biasAfter = tuple2i(0,0));
	
	Texture * loadOrGenerateBumpHeightMap(const std::string& heightMapFileName);
	Texture * loadNormalMap(const std::string& normalMapFileName, bool buildHeightComponent);

	void setAsActive();
	static TextureStorage * Active();

	static const char * NativeTextureExtension();

	ImageLoader& getImageLoader() { return mImageLoader; }

protected:
	RenderData::Image* loadImage(Data * data);
	RenderData::Image::Compression checkForceCompression(const RenderData::Image * image);

	Texture * loadFromImage(RenderData::Image * image, bool dontBuildMipmaps = false, bool dontCompress = false);

	virtual Texture* load(Data * data);
	virtual bool save(Texture* resource, Data * data, std::string& fileName);

	ImageLoader mImageLoader;
	
	float mHeightMapMultiplier;

	bool mPreferNativeTextures;

	bool mDontBuildMipmaps;
	bool mDontCompress;
};


}//namespace Resource {
}//namespace Squirrel {