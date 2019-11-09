#pragma once

#include <Common/types.h>
#include "ResourceStorage.h"
#include <Render/Image.h>
#include <Render/ITexture.h>

namespace Squirrel {

namespace Resource { 

class SQRESOURCE_API Texture:
	public StoredObject
{
	Render::ITexture * mRenderTexture;
	RenderData::Image * mSrcImage;

public:
	Texture(void);
	Texture(Render::ITexture * renderTexture);
	Texture(RenderData::Image * srcImage);
	Texture(RenderData::Image * srcImage, RenderData::Image::Compression forceCompress);
	Texture(RenderData::Image * srcImage, RenderData::Image::Compression forceCompress, bool genMipmap);
	virtual ~Texture(void);

	Render::ITexture *	getRenderTexture(void)	{ return mRenderTexture; }
	RenderData::Image *	getSrcImage(void)		{ return mSrcImage; }

	void deleteSrcImage();

private:

	void init(RenderData::Image * srcImage, RenderData::Image::Compression forceCompress, bool genMipmap);
};


}//namespace Resource { 

}//namespace Squirrel {