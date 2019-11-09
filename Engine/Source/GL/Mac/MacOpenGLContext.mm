#include "MacOpenGLContext.h"
#include <Common/Mac/MacWindow.h>
#include <Common/Mac/MacWindowManager.h>
#include <Common/Log.h>
#include <Math/BasicUtils.h>
#include "../Utils.h"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

namespace Squirrel {
namespace GL {
	
MacOpenGLContext::MacOpenGLContext(const PixelFormat& pf):
	Context(pf)
{
	mNSOpenGLContext = NULL;
	mNSOpenGLPixelFormat = NULL;
}

MacOpenGLContext::MacOpenGLContext(const PixelFormat& pf, Window * window):
	Context(pf)
{
	MacWindow * macWindow = static_cast<MacWindow *>(window);
	
	/*
	mPixelFormat.depthBits = Math::minValue(getCaps().depthBits, mPixelFormat.depthBits);
	mPixelFormat.stencilBits = Math::minValue(getCaps().stencilBits, mPixelFormat.stencilBits);
	mPixelFormat.colorBits = Math::minValue(getCaps().colorBits, mPixelFormat.colorBits);
	mPixelFormat.alphaBits = Math::minValue(getCaps().alphaBits, mPixelFormat.alphaBits);
	mPixelFormat.samples = Math::minValue(getCaps().multiSampleLevel, mPixelFormat.samples);
	 */
	
	ASSERT(macWindow != NULL);
	
	mNSOpenGLContext = NULL;
	mNSOpenGLPixelFormat = NULL;

    NSOpenGLPixelFormatAttribute attribs[] =
    {
		kCGLPFAAccelerated,
		kCGLPFANoRecovery,
		kCGLPFABackingStore,
		kCGLPFADoubleBuffer,
		//kCGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,
		//kCGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)mPixelFormat.alphaBits,
		kCGLPFADepthSize, (NSOpenGLPixelFormatAttribute)mPixelFormat.depthBits,
		//kCGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)mPixelFormat.stencilBits,
		//kCGLPFASampleBuffers, 1,
		//kCGLPFASamples, (NSOpenGLPixelFormatAttribute)mPixelFormat.samples,
		//kCGLPFASampleAlpha,
		//NSOpenGLPFASupersample
		// Must specify the 3.2 Core Profile to use OpenGL 3.2
#if SUPPORT_GL3
		kCGLPFAOpenGLProfile,
		kCGLOGLPVersion_3_2_Core,
#endif
		0
    };
	
    mNSOpenGLPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	
    if (!mNSOpenGLPixelFormat)
		NSLog(@"Failed to create opengl pixel format!");
	
	// NSOpenGLView does not handle context sharing, so we draw to a custom NSView instead
	mNSOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:(NSOpenGLPixelFormat *)mNSOpenGLPixelFormat shareContext:nil];
	
	NSView * nsView = (NSView *)macWindow->getWindowView();
	
	[(NSOpenGLContext *)mNSOpenGLContext setView:nsView];

	window->registerContext(this);
}

MacOpenGLContext::~MacOpenGLContext()
{
	if(mNSOpenGLPixelFormat)
		[(NSOpenGLPixelFormat *)mNSOpenGLPixelFormat release];
	if(mNSOpenGLContext)
		[(NSOpenGLContext *)mNSOpenGLContext release];
}

void MacOpenGLContext::activate()
{
	[(NSOpenGLContext *)this->mNSOpenGLContext makeCurrentContext];
}

void MacOpenGLContext::deactivate()
{
	[NSOpenGLContext clearCurrentContext];
}

Context * MacOpenGLContext::createSibling()	const
{
	NSOpenGLPixelFormat *clonePf = [(NSOpenGLPixelFormat *)this->mNSOpenGLPixelFormat retain];
	
	NSOpenGLContext *cloneCtx = [[NSOpenGLContext alloc] initWithFormat:clonePf
														   shareContext:(NSOpenGLContext *)this->mNSOpenGLContext];
	
	//Deprecated in OS X 10.8
	[cloneCtx copyAttributesFromContext:(NSOpenGLContext *)this->mNSOpenGLContext withMask:GL_ALL_ATTRIB_BITS];
	
	MacOpenGLContext * ret = new MacOpenGLContext(mPixelFormat);
	
	ret->mNSOpenGLContext = cloneCtx;
	ret->mNSOpenGLPixelFormat = clonePf;
	
	return ret;
}

void MacOpenGLContext::setSwapInterval(int interval)
{
	// Synchronize buffer swaps with vertical refresh rate
	[(NSOpenGLContext *)this->mNSOpenGLContext setValues:&interval forParameter:NSOpenGLCPSwapInterval];
}

void MacOpenGLContext::finishFrame()
{	
	// We draw on a secondary thread through the display link
	// When resizing the view, -reshape is called automatically on the main thread
	// Add a mutex around to avoid the threads accessing the context simultaneously	when resizing
	CGLLockContext((CGLContextObj)[(NSOpenGLContext *)this->mNSOpenGLContext CGLContextObj]);
	CGLFlushDrawable((CGLContextObj)[(NSOpenGLContext *)this->mNSOpenGLContext CGLContextObj]);
	CGLUnlockContext((CGLContextObj)[(NSOpenGLContext *)this->mNSOpenGLContext CGLContextObj]);
	
	//TODO: optimize it, update only when window is sizing
	[(NSOpenGLContext *)this->mNSOpenGLContext update];
	
	//[(NSOpenGLContext *)this->mNSOpenGLContext flushBuffer];
}

bool MacOpenGLContext::initCaps()
{
	CGDirectDisplayID mainDisplayId = static_cast<MacWindowManager *>(MacWindowManager::ActiveWindowManager())->getDisplayId();
	
	CGOpenGLDisplayMask cglDisplayMask = CGDisplayIDToOpenGLDisplayMask(mainDisplayId);

	CGLRendererInfoObj info;
	GLint j, numRenderers = 0, rv = 0;
	
	CGLError cglErr = CGLQueryRendererInfo ((GLuint)cglDisplayMask,
									&info,
									(GLint*)&numRenderers);
	if(kCGLNoError == cglErr) {
		CGLDescribeRenderer (info, 0, kCGLRPRendererCount, &numRenderers);
		for (j = 0; j < numRenderers; j++) {
			// find accelerated renderer (assume only one)
			CGLDescribeRenderer (info, j, kCGLRPAccelerated, &rv);
			if (true == rv) { // if accelerated
				
				CGLDescribeRenderer (info, j, kCGLRPMaxSamples, &mCaps.multiSampleLevel);
				
				int modes = 0;
				CGLDescribeRenderer (info, j, kCGLRPStencilModes, &modes);
				mCaps.stencilBits = (modes & kCGL8Bit) ? 8 : 0;
				CGLDescribeRenderer (info, j, kCGLRPDepthModes, &modes);
				mCaps.depthBits =	(modes & kCGL32Bit) ? 32 :
									((modes & kCGL24Bit) ? 24 :
									((modes & kCGL16Bit) ? 16 : 0));
					
				mCaps.colorBits = 24;
				mCaps.alphaBits = 8;
				mCaps.accumBits = 24;
				
				// what is the VRAM?
				CGLDescribeRenderer (info, j, kCGLRPVideoMemoryMegabytes, &mCaps.videoMemoryMegabytes);
				// what is the current texture memory?
				CGLDescribeRenderer (info, j, kCGLRPTextureMemoryMegabytes, &mCaps.textureMemoryMegabytes);
				break; // done
			}
		}
	}
	CGLDestroyRendererInfo (info);

	if(Utils::IsExtensionSupported("EXT_texture_filter_anisotropic"))
	{
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mCaps.maxAnisotropy);
		CHECK_GL_ERROR;
	}
	else
	{
		mCaps.maxAnisotropy = 1.0f;
	}
	
	tuple2i version = Utils::GetGLVersion();

	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&mCaps.max2DTextureSize);
	CHECK_GL_ERROR;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE,&mCaps.max3DTextureSize);
	CHECK_GL_ERROR;
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE,&mCaps.maxCubeTextureSize);
	CHECK_GL_ERROR;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&mCaps.maxTextureUnits);
	CHECK_GL_ERROR;
	
	//print info

	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tOpenGL vendor: " << glGetString(GL_VENDOR);
	CHECK_GL_ERROR;
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tOpenGL render: " << glGetString(GL_RENDERER);
	CHECK_GL_ERROR;
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tOpenGL version: " << version.x << "." << version.y;

	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tVideo memory (Mb): " << mCaps.videoMemoryMegabytes;
	
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tMax texture size: " << mCaps.max2DTextureSize;
	
	return true;
}

}//namespace GL {
}//namespace Squirrel {