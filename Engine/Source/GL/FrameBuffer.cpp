// FrameBuffer.cpp: implementation of the FrameBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "FrameBuffer.h"
#include "Texture.h"
#include <common/Log.h>
#include "Utils.h"

namespace Squirrel {

namespace GL {


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer( int theWidth, int theHeight, int theFlags )
{
	mWidth         = theWidth;
	mHeight        = theHeight;
	mFlags         = theFlags;
	mFrameBuffer   = 0;
	mColorAttachemntsNum = -1;
}

FrameBuffer::~FrameBuffer()
{
	setPool(NULL);
}

void FrameBuffer :: generate()
{
	glGenFramebuffers( 1, &mFrameBuffer );
}

void FrameBuffer :: destroy()
{
	if ( mDepthBuffer.id != 0 )
		glDeleteRenderbuffers( 1, &mDepthBuffer.id );
	if ( mStencilBuffer.id != 0 )
		glDeleteRenderbuffers( 1, &mStencilBuffer.id );
	if ( mFrameBuffer != 0 )
		glDeleteFramebuffers( 1, &mFrameBuffer );
}

bool	FrameBuffer :: create ()
{
	if ( mWidth <= 0 || mHeight <= 0 )// || width >= GL_MAX_RENDERBUFFER_SIZE || height >= GL_MAX_RENDERBUFFER_SIZE )
		return false;

	glBindFramebuffer( GL_FRAMEBUFFER, mFrameBuffer );
	CHECK_GL_ERROR;
	//sBoundFramebuffer = this;

	int	depthFormat   = 0;
	int stencilFormat = 0;

	if ( mFlags & depth16 )
		depthFormat = GL_DEPTH_COMPONENT16;
	else
	if ( mFlags & depth24 )
		depthFormat = GL_DEPTH_COMPONENT24;
	else
	if ( mFlags & depth32 )
		depthFormat = GL_DEPTH_COMPONENT32;
	
	if ( mFlags & stencil1 )
		stencilFormat = GL_STENCIL_INDEX1;
	else
	if ( mFlags & stencil4 )
		stencilFormat = GL_STENCIL_INDEX4;
	else
	if ( mFlags & stencil8 )
		stencilFormat = GL_STENCIL_INDEX8;
	else
	if ( mFlags & stencil16 )
		stencilFormat = GL_STENCIL_INDEX16;

	if ( depthFormat != 0 )
	{
		glGenRenderbuffers        ( 1, &mDepthBuffer.id );
		glBindRenderbuffer        ( GL_RENDERBUFFER, mDepthBuffer.id );
		glRenderbufferStorage     ( GL_RENDERBUFFER, depthFormat, mWidth, mHeight );
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer.id );
		CHECK_GL_ERROR;
	}
	
	if ( stencilFormat != 0 )
	{
		glGenRenderbuffers        ( 1, &mStencilBuffer.id );
		glBindRenderbuffer        ( GL_RENDERBUFFER, mStencilBuffer.id );
		glRenderbufferStorage     ( GL_RENDERBUFFER, stencilFormat, mWidth, mHeight );
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_STENCIL_ATTACHMENT,
		                               GL_RENDERBUFFER, mStencilBuffer.id );
		CHECK_GL_ERROR;
	}
	
	setColorAttachmentsNum(0);

	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	CHECK_GL_ERROR;

	return true;
}

bool	FrameBuffer :: isOk () const
{
	GLint	currentFb;
	glGetIntegerv ( GL_FRAMEBUFFER_BINDING, (int *)&currentFb );
	if ( currentFb != mFrameBuffer )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, mFrameBuffer );
		CHECK_GL_ERROR;
	}
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	bool	complete = status == GL_FRAMEBUFFER_COMPLETE;

	if(status==GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT )			
		Log::Instance().warning("FrameBuffer", "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
	if(status==GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT )	
		Log::Instance().warning("FrameBuffer", "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
	if(status==GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)			
		Log::Instance().warning("FrameBuffer", "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
	if(status==GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)			
		Log::Instance().warning("FrameBuffer", "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
	if(status==GL_FRAMEBUFFER_UNSUPPORTED)						
		Log::Instance().warning("FrameBuffer", "GL_FRAMEBUFFER_UNSUPPORTED");

	return complete;
}

bool FrameBuffer::bind ()
{
	if(sBoundFramebuffer == this)
		return true;

	if ( mFrameBuffer == 0 )
	{
		Log::Instance().error("FrameBuffer::bind", "frameBuffer == 0");
		return false;
	}

	glBindFramebuffer	( GL_FRAMEBUFFER, mFrameBuffer );
	CHECK_GL_ERROR;
	sBoundFramebuffer	= this;
	
	glViewport			( 0, 0, getWidth (), getHeight () );
	CHECK_GL_ERROR;

	return true;
}

void FrameBuffer::UnbindAll ()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	sBoundFramebuffer = NULL;
}

bool	FrameBuffer :: unbind ()
{
	if ( mFrameBuffer == 0 )
		return false;
	
	UnbindAll();

	return true;
}
	
void FrameBuffer :: setColorAttachmentsNum(int colorAttachmentsNum)
{
	if(mColorAttachemntsNum == colorAttachmentsNum)
		return;
	
	const GLenum colorDrawBuffers[sMaxColorAttachments] = {
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7
	};
	
	//set read buffer
	glReadBuffer		( colorAttachmentsNum > 0 ? GL_COLOR_ATTACHMENT0 : GL_NONE );
	CHECK_GL_ERROR;
	
	//set draw buffer(s)
	if(colorAttachmentsNum < 2)
	{
		glDrawBuffer	( colorAttachmentsNum > 0 ? GL_COLOR_ATTACHMENT0 : GL_NONE );
	}
	else
	{
		glDrawBuffers	( colorAttachmentsNum, colorDrawBuffers );
	}	
	CHECK_GL_ERROR;
	
	mColorAttachemntsNum = colorAttachmentsNum;
}

bool attachTexture(GLenum attachment, Texture * oglTex, int level = 0)
{
	if ( oglTex->getTarget() != GL_TEXTURE_2D && oglTex->getTarget() != Utils::getTexRectTarget())
		return false;
	
	glFramebufferTexture2D ( GL_FRAMEBUFFER, attachment, oglTex->getTarget(), oglTex->getTexId(), 0 );
	CHECK_GL_ERROR;
	
	return true;
}

bool attachTextureFace(GLenum attachment, Texture * oglTex, int face, int level = 0)
{
	if(oglTex->getTarget() == GL_TEXTURE_CUBE_MAP)
	{
		glFramebufferTexture2D ( GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, oglTex->getTexId(), level );
	}
	else if(oglTex->getTarget() == GL_TEXTURE_2D_ARRAY)
	{
		glFramebufferTextureLayer ( GL_FRAMEBUFFER, attachment, oglTex->getTexId(), level, face );
	}
	else if(oglTex->getTarget() == GL_TEXTURE_3D)
	{
		glFramebufferTexture3D ( GL_FRAMEBUFFER, attachment, GL_TEXTURE_3D, oglTex->getTexId(), level, face );
	}
	else
		return false;
	
	CHECK_GL_ERROR;
	return true;
}	

bool FrameBuffer :: dettachColor(int no)
{
	glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_COLOR_ATTACHMENT0 + no, GL_RENDERBUFFER, 0 );
	mColorBuffer[no].texture	= NULL;
	mColorBuffer[no].id		= 0;
	return true;
}

bool FrameBuffer :: dettachDepth()
{
	glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
	mDepthBuffer.texture	= NULL;
	mDepthBuffer.id		= 0;
	return true;
}

ITexture * FrameBuffer :: getAttachement(int no)
{
	return mColorBuffer[no].texture;
}
	
bool FrameBuffer :: attachColorTexture ( ITexture * tex, int no )
{
	if ( mFrameBuffer == 0 )
		return false;

	if(	tex == NULL )
		return dettachColor(no);

	Texture * oglTex = static_cast<Texture *>(tex);
	
	if(attachTexture(GL_COLOR_ATTACHMENT0 + no, oglTex))
	{
		mColorBuffer[no].texture = oglTex;
		mColorBuffer[no].id = 0;
		
		if(no == 0 && mColorAttachemntsNum < 1)
			setColorAttachmentsNum(1);
		
		return true;
	}
	
	return false;
}
		
bool FrameBuffer :: attachColorTextureFace ( ITexture * tex, int face, int no )
{
	if ( mFrameBuffer == 0 )
		return false;
	
	if(	tex == NULL )
		return dettachColor(no);
	
	Texture * oglTex = static_cast<Texture *>(tex);
	
	if(attachTextureFace(GL_COLOR_ATTACHMENT0 + no, oglTex, face))
	{
		mColorBuffer[no].texture = oglTex;
		mColorBuffer[no].id = 0;
		
		if(no == 0 && mColorAttachemntsNum < 1)
			setColorAttachmentsNum(1);
		
		return true;
	}
	
	return false;
}
	
bool FrameBuffer :: attachDepthTextureFace ( ITexture * tex, int face )
{
	if ( mFrameBuffer == 0 )
		return false;
	
	if(	tex == NULL )
		return dettachDepth();
	
	Texture * oglTex = static_cast<Texture *>(tex);

	if(attachTextureFace(GL_DEPTH_ATTACHMENT, oglTex, face))
	{
		mDepthBuffer.texture = oglTex;
		mDepthBuffer.id = 0;
		return true;
	}
	
	return false;
}
	
bool FrameBuffer :: attachDepthTexture ( ITexture * tex )
{
	if ( mFrameBuffer == 0 )
		return false;
	
	if(	tex == NULL )
		return dettachDepth();
	
	Texture * oglTex = static_cast<Texture *>(tex);
	
	if(attachTexture(GL_DEPTH_ATTACHMENT, oglTex))
	{
		mDepthBuffer.texture = oglTex;
		mDepthBuffer.id = 0;
		return true;
	}
		
	return false;
}

bool	FrameBuffer :: createDepthBuffer ( int flags )
{
	if ( mFrameBuffer == 0 )
		return false;

	if(sBoundFramebuffer != this)
		bind();

	if(mDepthBuffer.id > 0 || mDepthBuffer.texture == NULL)
	{
		attachDepthTexture(NULL);
	}

	int	depthFormat   = 0;

	if ( flags & depth16 )
		depthFormat = GL_DEPTH_COMPONENT16;
	else
	if ( flags & depth24 )
		depthFormat = GL_DEPTH_COMPONENT24;
	else
	if ( flags & depth32 )
		depthFormat = GL_DEPTH_COMPONENT32;
	
	if ( depthFormat != 0 )
	{
		glGenRenderbuffers        ( 1, &mDepthBuffer.id );
		glBindRenderbuffer        ( GL_RENDERBUFFER, mDepthBuffer.id );
		glRenderbufferStorage     ( GL_RENDERBUFFER, depthFormat, mWidth, mHeight );
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT,
		                               GL_RENDERBUFFER, mDepthBuffer.id );
		CHECK_GL_ERROR;
	}

	return true;
}

int FrameBuffer::MaxColorAttachemnts ()
{
    int n;
    glGetIntegerv ( GL_MAX_COLOR_ATTACHMENTS, &n );
	return n;
}

bool FrameBuffer::IsSupported ()
{
	return Utils::GetGLVersion().x >= 2;
}

void FrameBuffer::buildMipmaps ( GLenum target ) const
{
	// be sure we're unbound
	getColorBuffer().texture->bind();
	glGenerateMipmap ( target );
	CHECK_GL_ERROR;
}

int	FrameBuffer::MaxSize ()
{
    GLint sz;
    glGetIntegerv ( GL_MAX_RENDERBUFFER_SIZE, &sz );
	return sz;
}

} //namespace GL {

} //namespace Squirrel {
