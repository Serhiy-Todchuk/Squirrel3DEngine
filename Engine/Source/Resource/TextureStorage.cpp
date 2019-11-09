#include "TextureStorage.h"
#include "ImageLoader.h"
#include <FileSystem/Path.h>
#include <Common/Settings.h>
#include <Common/Log.h>
#include <Math/BasicUtils.h>
#include <algorithm>
#include <cctype>

namespace Squirrel {

namespace Resource { 

using namespace RenderData;

TextureStorage * TextureStorage::sActiveLibrary = NULL;

TextureStorage * TextureStorage::Active()
{
	return sActiveLibrary;
}

const char * TextureStorage::NativeTextureExtension()
{
	return "sqtex";
}

TextureStorage::TextureStorage()
{
	mDontBuildMipmaps = false;
	mDontCompress = false;

	mPreferNativeTextures = Settings::Default()->getInt("Resources", "PreferNativeTextures", 1) != 0;
	mHeightMapMultiplier = Settings::Default()->getFloat("Resources", "Height2NormalMapScale", 6.4f);
}

TextureStorage::~TextureStorage()
{
	if(this == sActiveLibrary)
	{
		sActiveLibrary = NULL;
	}
}

Texture * TextureStorage::loadTexture(const std::string& texName, bool dontBuildMipmaps, bool dontCompress)
{
	Texture * tex = NULL;

	mDontBuildMipmaps = dontBuildMipmaps;
	mDontCompress = dontCompress;
	
	//try to load texture with the same name but native format if format is not native
	if(mPreferNativeTextures && FileSystem::Path::GetExtension(texName) != TextureStorage::NativeTextureExtension())
	{
		std::string resourceName = FileSystem::Path::RemoveExtension(texName) + "." + TextureStorage::NativeTextureExtension();
		tex = add( resourceName );
	}

	//if texture is not loaded yet - do regular loading
	if(tex == NULL)
	{
		tex = add( texName );
	}
	
	mDontBuildMipmaps = false;
	mDontCompress = false;

	return tex;
}

Texture * TextureStorage::makeCubemap(std::string fileNames[Render::ITexture::cmfNum])
{
	Image * cubeImage = NULL;

	bool isOk = false;

	for(int faceIndex = 0; faceIndex < Render::ITexture::cmfNum; ++faceIndex)
	{
		Data * faceData = getResourceData(fileNames[faceIndex]);
		if(faceData == NULL) break;

		Image * faceImage = loadImage(faceData);
		if(faceImage == NULL) break;

		DELETE_PTR( faceData );

		if(cubeImage == NULL)
		{
			cubeImage = new Image(faceImage->getWidth(), faceImage->getHeight(), 1, 
				faceImage->getDataType(), faceImage->getFormat(), faceImage->getCompression(), 1, true);
		}
		else
		{
			if(faceImage->getWidth() != faceImage->getHeight()) break;

			//do comparison here
		}

		Image::Level& faceLevel = cubeImage->getLevel(0, faceIndex);
		Image::Level& srcLevel = faceImage->getLevel(0, 0);

		if(faceLevel.size != srcLevel.size) break;

		memcpy(faceLevel.data, srcLevel.data, srcLevel.size);

		if(faceIndex == Render::ITexture::cmfNum - 1) isOk = true;
	}

	if(!isOk)
	{
		DELETE_PTR(cubeImage);
		return NULL;
	}

	Texture * pTex = new Texture(cubeImage, checkForceCompression(cubeImage));
	ASSERT(pTex);

	return pTex;
}
	
Texture * TextureStorage::makeGridAtlas(const std::list<std::string>& fileNames, tuple2i atlasSize, tuple2i biasBefore,	tuple2i biasAfter)
{
	//determine grid size
	
	int cellsNum = Math::getNextPowerOfTwo((int)fileNames.size());
	size_t gridSize = (size_t)sqrt((double)cellsNum);
	
	size_t cellImageWidth	= (atlasSize.x / gridSize) - (biasBefore.x + biasAfter.x);
	size_t cellImageHeight	= (atlasSize.y / gridSize) - (biasBefore.y + biasAfter.y);
	
	if(gridSize == 0)
		return NULL;
	
	tuple2i gridPos(0, 0);
	
	Image * atlasImage = NULL;
	
	FOREACH(std::list<std::string>::const_iterator, itName, fileNames)
	{		
		const std::string& fileName = (*itName);
		
		Data * cellData = getResourceData(fileName);
		if(cellData == NULL)
			continue;
		
		Image * cellImage = loadImage(cellData);
		DELETE_PTR( cellData );
		
		if(cellImage == NULL)
			continue;
		
		if(atlasImage == NULL)
		{
			atlasImage = new Image(atlasSize.x, atlasSize.y, 1, cellImage->getDataType(), cellImage->getFormat());
		}
		
		ASSERT(false);//not implemented
		
		if(++gridPos.x >= gridSize)
		{
			++gridPos.y;
		}
	}
	
	Texture * pTex = new Texture(atlasImage, checkForceCompression(atlasImage));
	return pTex;
}
	
Texture * TextureStorage::loadOrGenerateBumpHeightMap(const std::string& heightMapFileName)
{
	std::string fname = FileSystem::Path::RemoveExtension( heightMapFileName );
	std::string newName = fname + std::string("_nhm.") + TextureStorage::NativeTextureExtension();
	
	//first try to load texture from disk if generated one is already existed
	Texture * tex = add( newName );
	if(!tex)
	{
		//otherwise generate it from heightMap/diffuse texture
		Data * imageSrc = TextureStorage::Active()->getResourceData( heightMapFileName );
		if(imageSrc)
		{
			Image * img		= TextureStorage::Active()->getImageLoader().loadImage( imageSrc ); ASSERT(img);
			Image * nhmImg	= img->buildNormalHeightMapFromHeight(mHeightMapMultiplier); ASSERT(nhmImg);
			
			tex = new Texture(nhmImg, Image::DXT5);
			ASSERT(tex);
			
			tex->setName( newName );
			
			TextureStorage::Active()->addNew(newName, tex);
			
			DELETE_PTR(img);
			DELETE_PTR(imageSrc);
		}
	}
	
	return tex;
}

Texture * TextureStorage::loadNormalMap(const std::string& normalMapFileName, bool buildHeightComponent)
{
	Data * imageSrc = getResourceData( normalMapFileName );
	if(imageSrc)
	{
		Image * img		= getImageLoader().loadImage( imageSrc ); ASSERT(img);
		img->setContentType(Image::Vectors);

		Image * nhm		= img->buildNormalHeightMapFromNormals();
		DELETE_PTR(img);

		Texture * tex = new Texture(nhm, Image::DXT5);
		ASSERT(tex);

		tex->setName( normalMapFileName );
		addNew(normalMapFileName, tex);

		DELETE_PTR(imageSrc);

		return tex;
	}

	return NULL;
}

Image* TextureStorage::loadImage(Data * data)
{
	bool nativeFormat = true;

	//detect native image format
	char sign[4];
	for(int i = 0; i < 4; ++i) 
	{
		sign[i] = data->readByte();
		if(sign[i] != Image::GetNativeFileSign()[i])
		{
			nativeFormat = false;
			break;
		}
	}
	//return position
	data->seekAbs(0);

	Image * pImage = NULL;

	//load image
	if(nativeFormat)
	{
		pImage = new Image();
		ASSERT(pImage->load(data));
	}
	else
	{
		pImage = mImageLoader.loadImage(data);
	}

	if(pImage == NULL)
	{
		Log::Instance().streamError("TextureStorage::loadImage") << "Failed to load image " << data->getFileName() << "!";
		Log::Instance().flush();
	}

	return pImage;
}

Image::Compression TextureStorage::checkForceCompression(const Image * image)
{
	Image::Compression compression = Image::Uncompressed;

	if(!image->isNative())
	{
		//force compress only textures with foreign format
		bool forceCompress = Settings::Default()->getInt("Resources", "ForceTextureCompress", 1) != 0;
		if(forceCompress && image->getCompression() == Image::Uncompressed)
		{
			if(image->getFormat() == Image::RGB)
				compression = Image::DXT1;
			else if(image->getFormat() == Image::RGBA)
				compression = Image::DXT5;
		}
	}

	return compression;
}

Texture* TextureStorage::load(Data * data)
{
	Image * pImage = loadImage(data);

	DELETE_PTR(data);

	if(pImage == NULL)
		return NULL;

	Texture * pTex = loadFromImage(pImage, mDontBuildMipmaps, mDontCompress);

	return pTex;
}


Texture * TextureStorage::loadFromImage(Image * image, bool dontBuildMipmaps, bool dontCompress)
{
	Image::Compression compression = dontCompress ? Image::Uncompressed : checkForceCompression(image);

	Texture * pTex = new Texture(image, compression, !dontBuildMipmaps);
	ASSERT(pTex);

	if(!pTex->isChanged())
		pTex->deleteSrcImage();

	return pTex;
}

bool TextureStorage::save(Texture* resource, Data * data, std::string& fileName)
{
	if(resource == NULL || resource->getSrcImage() == NULL || data == NULL) 
	{
		return false;
	}

	//change extension
	std::string ext = FileSystem::Path::GetExtension( fileName );
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if(ext != NativeTextureExtension())
	{
		fileName = FileSystem::Path::RemoveExtension( fileName ) + "." + NativeTextureExtension();
	}

	return resource->getSrcImage()->save(data);
}

void TextureStorage::setAsActive()
{
	sActiveLibrary = this;
}

void TextureStorage::deleteSourceImages(bool keepChanged)
{
	for(_ID i = 0; i < getSize(); ++i)
	{
		Texture * obj = get(i);
		if(obj)
		{
			if( obj->isChanged() && keepChanged ) continue;

			obj->deleteSrcImage();
		}
	}
}

}//namespace Resource { 

}//namespace Squirrel {