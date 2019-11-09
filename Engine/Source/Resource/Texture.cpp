#include "Texture.h"
#include <Render/IRender.h>
#include <Common/Settings.h>
#include <Common/Log.h>

namespace Squirrel {

namespace Resource { 

using namespace Render;

Texture::Texture(void)
{
	mRenderTexture = NULL;
}

Texture::Texture(Render::ITexture * renderTexture)
{
	mRenderTexture = renderTexture;
}

Texture::Texture(RenderData::Image * srcImage)
{
	init(srcImage, RenderData::Image::Uncompressed, true);
}

Texture::Texture(RenderData::Image * srcImage, RenderData::Image::Compression forceCompress)
{
	init(srcImage, forceCompress, true);
}
	
Texture::Texture(RenderData::Image * srcImage, RenderData::Image::Compression forceCompress, bool genMipmap)
{
	init(srcImage, forceCompress, genMipmap);
}

Texture::~Texture(void)
{
	DELETE_PTR(mRenderTexture);
	deleteSrcImage();
}

void Texture::init(RenderData::Image * srcImage, RenderData::Image::Compression forceCompress, bool genMipmap)
{
	//create empty texture
	ASSERT(IRender::GetActive());
	mRenderTexture = IRender::GetActive()->createTexture();
	ASSERT(mRenderTexture != NULL);

	//build mipmaps if needed
	bool forceMipmapGen = Settings::Default()->getInt("Resources", "ForceMipmapGen", 1) != 0;
	if(srcImage->getLevelsNum() == 1 && forceMipmapGen && genMipmap)
	{
		if(!srcImage->buildMipMaps())
		{
			Log::Instance().warning("Resources::Texture::init", "Failed to build mipmaps! Continue loading texture without mipmaps.");
		}
		else
		{
			setChanged();
		}
	}

	//fill render texture with image data
	if(!mRenderTexture->fill(srcImage, forceCompress))
	{
		Log::Instance().error("Resources::Texture::init", "Failed to fill render texture with image data!");
		return;
	}

	mSrcImage = srcImage;
	if(forceCompress != RenderData::Image::Uncompressed)
	{
		//image has been forcibly compressed
		//obtain new image from GPU
		srcImage = mRenderTexture->getImage();
		if(srcImage != NULL)
		{
			deleteSrcImage();
			mSrcImage = srcImage;
			setChanged();
		}
		else
		{
			Log::Instance().warning("Resources::Texture::init", "Failed to obtain compressed image data from GPU!");
		}
	}
}

void Texture::deleteSrcImage() 
{ 
	DELETE_PTR( mSrcImage ); 
}



}//namespace Resource { 

}//namespace Squirrel {