// FrameBuffer.h: interface for the FrameBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "macros.h"
#include "Utils.h"
#include "IContextObject.h"
#include "ITexture.h"

namespace Squirrel {

namespace Render { 

class SQRENDER_API IFrameBuffer  :
	public IContextObject
{
protected:
	int		mFlags;
	int		mWidth;
	int		mHeight;
	
public:
	IFrameBuffer  () {};
	virtual ~IFrameBuffer () {};
	
	int	getWidth () const { return mWidth; }
	int	getHeight() const { return mHeight; }
	
	virtual bool	isOk   () const			= 0;
	virtual bool	create ()				= 0;
	virtual bool	bind   ()				= 0;
	virtual bool	unbind   ()				= 0;

	virtual bool attachColorTexture ( ITexture * tex, int no = 0 )	= 0;
	virtual bool attachDepthTexture ( ITexture * tex )	= 0;
	virtual bool attachColorTextureFace ( ITexture * tex, int face, int no = 0 )	= 0;
	virtual bool attachDepthTextureFace ( ITexture * tex, int face )	= 0;
	
	virtual void setColorAttachmentsNum(int colorAttachmentsNum) = 0;

	virtual ITexture * getAttachement(int no = 0) = 0;
		
	enum								// flags for depth and stencil buffers
	{
		depth16 = 1,					// 16-bit depth buffer
		depth24 = 2,					// 24-bit depth buffer
		depth32 = 4,					// 32-bit depth buffer
		
		stencil1  = 16,					// 1-bit stencil buffer
		stencil4  = 32,					// 4-bit stencil buffer
		stencil8  = 64,					// 8-bit stencil buffer
		stencil16 = 128					// 16-bit stencil buffer
	};
	
	enum								// filter modes
	{
		filterNearest = 0,
		filterLinear  = 1,
		filterMipmap  = 2
	};

	static IFrameBuffer * GetBoundFrameBuffer() { return sBoundFramebuffer; }

	static void Unbind() { 
		IFrameBuffer * fb = GetBoundFrameBuffer();
		if(fb != NULL)
			fb->unbind();
	}

protected:

	static IFrameBuffer * sBoundFramebuffer;

};


} //namespace Render {

} //namespace Squirrel {
