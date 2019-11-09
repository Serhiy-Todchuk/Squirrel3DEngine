// FrameBuffer.h: interface for the FrameBuffer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <Render/IFrameBuffer.h>
#include <GL/Texture.h>
#include "Utils.h"

namespace Squirrel {

namespace GL { 

using namespace Render;

class SQOPENGL_API FrameBuffer:
	public IFrameBuffer
{
public:
	struct Attachment
	{
		Attachment(): id(0), layer(0), level(0), texture(NULL) {}

		GLuint id;//id of render buffer

		GLuint layer;//for 3D texture, texture array and cubemap
		GLuint level;//mipmap level
		Texture * texture;
	};

	static const int sMaxColorAttachments = 8;

private:
	GLuint		mFrameBuffer;					// id of framebuffer object
	Attachment	mColorBuffer [sMaxColorAttachments];				// texture id or buffer id
	Attachment	mDepthBuffer;
	Attachment	mStencilBuffer;					//
	int			mColorAttachemntsNum;
	
public:
	FrameBuffer  ( int theWidth, int theHeight, int theFlags = 0 );
	~FrameBuffer ();
	
	bool hasStencil() const { return mStencilBuffer.id != 0 || mStencilBuffer.texture; }
	bool hasDepth  () const { return mDepthBuffer.id != 0 || mDepthBuffer.texture; }
	
	bool	isOk   () const;
	bool	create ();
	bool	bind   ();
	bool	unbind   ();

	virtual bool attachColorTexture ( ITexture * tex, int no = 0 );
	virtual bool attachDepthTexture ( ITexture * tex );
	virtual bool attachColorTextureFace ( ITexture * tex, int face, int no = 0 );
	virtual bool attachDepthTextureFace ( ITexture * tex, int face );
	
	virtual void setColorAttachmentsNum(int colorAttachmentsNum);

	virtual ITexture * getAttachement(int no = 0);

	bool createDepthBuffer  ( int flags );
	
	const Attachment&	getColorBuffer ( int no = 0 ) const	{	return mColorBuffer [no];	}
	const Attachment&	getDepthBuffer () const				{	return mDepthBuffer;	}
	const Attachment&	getStencilBuffer () const			{	return mStencilBuffer;	}
										 
	//implement IContextObject
	virtual void generate();
	virtual void destroy();

	// mipmapping support
	void	buildMipmaps ( GLenum target = GL_TEXTURE_2D ) const;
	
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
	
	static	void	UnbindAll ();
	static	bool	IsSupported         ();
	static	int		MaxColorAttachemnts ();
	static	int		MaxSize             ();
	
private:
	bool dettachColor(int no = 0);
	bool dettachDepth();
};


} //namespace GL {

} //namespace Squirrel {
