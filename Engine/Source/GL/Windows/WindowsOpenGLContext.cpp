#include "WindowsOpenGLContext.h"
#include <Common/Windows/WindowsWindow.h>
#include <Common/Log.h>
#include <Math/BasicUtils.h>
#include "../Utils.h"

namespace Squirrel {
namespace GL {

WindowsOpenGLContext::WindowsOpenGLContext(const PixelFormat& pf):
	Context(pf)
{
}

WindowsOpenGLContext::WindowsOpenGLContext(const PixelFormat& pf, Window * window):
	Context(pf)
{
	WindowsWindow * windowsWindow = static_cast<WindowsWindow *>(window);

	ASSERT(windowsWindow != NULL);

	mDeviceContextHandle = windowsWindow->getDeviceContextHandle();

	mIsPixelFormatDescribed = false;

	if(!setAdvancedPixelFormat())
	{
		setPixelFormat();
	}

	if (SetPixelFormat( mDeviceContextHandle, mPixelFormatIndex, mIsPixelFormatDescribed ? &mPixelFormatDescription : NULL) == FALSE)
	{
		Log::Instance().error("WindowsOpenGLContext", "Failed to set pixel format!"); 
		mRenderingContextHandle = NULL;
		return;
	}

	mRenderingContextHandle = wglCreateContext(mDeviceContextHandle);

	window->registerContext(this);
}

WindowsOpenGLContext::~WindowsOpenGLContext()
{
	if(mRenderingContextHandle != NULL)
		wglDeleteContext (mRenderingContextHandle);
}

void WindowsOpenGLContext::activate()
{
	wglMakeCurrent(mDeviceContextHandle, mRenderingContextHandle);
}

void WindowsOpenGLContext::deactivate()
{
	wglMakeCurrent(NULL, NULL);
}

Context * WindowsOpenGLContext::createSibling()	const
{
	WindowsOpenGLContext * ret = new WindowsOpenGLContext(mPixelFormat);

	ret->mDeviceContextHandle = mDeviceContextHandle;

	// Create new context based on own HDC
	ret->mRenderingContextHandle = wglCreateContext(mDeviceContextHandle);
	
	ASSERT(ret->mRenderingContextHandle != NULL)

	HGLRC oldrc = wglGetCurrentContext();
	HDC oldhdc = wglGetCurrentDC();

	wglMakeCurrent(NULL, NULL);

	// Share lists with old context
	if (!wglShareLists(mRenderingContextHandle, ret->mRenderingContextHandle))
	{
		//String errorMsg = translateWGLError();
		//TODO: throw exception
		delete ret;
	}

	// restore old context
	wglMakeCurrent(oldhdc, oldrc);
	
	return ret;
}

void WindowsOpenGLContext::setSwapInterval(int interval)
{
	wglSwapIntervalEXT(interval);
}

void WindowsOpenGLContext::finishFrame()
{
	SwapBuffers(mDeviceContextHandle);
}

bool WindowsOpenGLContext::initCaps()
{
	if(!GLEE_WGL_ARB_pixel_format)
	{
		return false;
	}

	int attributes[11];
	int results[11];

	attributes[0]  = WGL_NUMBER_PIXEL_FORMATS_ARB;
	wglGetPixelFormatAttribivARB(mDeviceContextHandle, 1, 0, 1, attributes, results);
	int    numPfds = results[0];

	//A list of attributes to check for each pixel format
	attributes[ 0]  =  WGL_COLOR_BITS_ARB;    //bits
	attributes[ 1]  =  WGL_ALPHA_BITS_ARB;
	attributes[ 2]  =  WGL_DEPTH_BITS_ARB;
	attributes[ 3]  =  WGL_STENCIL_BITS_ARB;

	attributes[ 4]  =  WGL_DRAW_TO_WINDOW_ARB;  //required to be true
	attributes[ 5]  =  WGL_SUPPORT_OPENGL_ARB;
	attributes[ 6]  =  WGL_DOUBLE_BUFFER_ARB;

	attributes[ 7]  =  WGL_ACCELERATION_ARB;  //required to be FULL_ACCELERATION_ARB

	attributes[ 8]  =  WGL_SAMPLE_BUFFERS_ARB;  //Multisample
	attributes[ 9]  =  WGL_SAMPLES_ARB;

	attributes[10]  =  WGL_ACCUM_BITS_ARB;

	for(int i=0; i<numPfds; ++i)
	{
		if(wglGetPixelFormatAttribivARB(mDeviceContextHandle, i+1, 0, 11, attributes, results))
		{
			mCaps.doubleBuffer=(results[6] ? true : mCaps.doubleBuffer);
			mCaps.multiSampleLevel=((mCaps.multiSampleLevel  < results[9]) ? results[9]   : mCaps.multiSampleLevel);
			//mCaps.accumulation=((mCaps.accumulation < results[10]) ? results[10]   : mCaps.accumulation);
			mCaps.stencilBits= ((mCaps.stencilBits  < results[ 3]) ? results[ 3]   : mCaps.stencilBits);
			mCaps.colorBits=   ((mCaps.colorBits*4  < results[ 0]) ? results[ 0]/4 : mCaps.colorBits);
			mCaps.alphaBits=   ((mCaps.alphaBits    < results[ 1]) ? results[ 1]   : mCaps.alphaBits);
			mCaps.depthBits=   ((mCaps.depthBits    < results[ 2]) ? results[ 2]   : mCaps.depthBits);
		}
	}

	if(GLEE_VERSION_1_3)
	{
		glGetIntegerv( GL_MAX_TEXTURE_UNITS, &mCaps.max2DTextureSize);//TODO: remove it as it is deprecated
	}

	if(GLEE_EXT_texture_filter_anisotropic)
	{
		glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mCaps.maxAnisotropy);
	}

	//clamp

	int arg = Math::clamp(mCaps.depthBits, 0, 24);
	switch(arg)
	{
	case  0:
	case  8:
	case 16:
	case 24:
		mCaps.depthBits = arg;
		break;
	default:
		mCaps.depthBits =	(arg <  8) ?  8 :
					(arg < 16) ? 16 : 24;
	}
	mCaps.stencilBits		= Math::clamp(mCaps.stencilBits, 0, 8);
	mCaps.colorBits		= Math::clamp(mCaps.colorBits,   0, 32);
	//mCaps.accumulation	= Math::clamp(mCaps.accumulation,0, 1024);
	mCaps.alphaBits		= Math::clamp(mCaps.alphaBits, 0, 8);
	arg				= Math::clamp(mCaps.multiSampleLevel, 0, 16);
	mCaps.multiSampleLevel = arg ? Math::getClosestPowerOfTwo(arg) : 0;

	//print info

	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tOpenGL vendor: " << glGetString(GL_VENDOR);
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tOpenGL render: " << glGetString(GL_RENDERER);
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tOpenGL version: " << glGetString(GL_VERSION);
	CHECK_GL_ERROR;

	int maxTextureSize = 0; 
	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTextureSize);
	CHECK_GL_ERROR;
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tGPU max texture size: " << maxTextureSize;

	int maxTextureUnitsNum = 0; 
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnitsNum);
	CHECK_GL_ERROR;
	Log::Instance().stream("GLCaps (init)", Log::sevInformation) << "\tGPU max texture units num: " << maxTextureUnitsNum;

	return true;
}

bool WindowsOpenGLContext::setPixelFormat()
{
	PIXELFORMATDESCRIPTOR& pfd = mPixelFormatDescription;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion    = 1;

    pfd.dwFlags   = PFD_DRAW_TO_WINDOW | 
                    PFD_SUPPORT_OPENGL | 
                    PFD_DOUBLEBUFFER;

    pfd.iPixelType     = PFD_TYPE_RGBA;
    pfd.cColorBits     = mPixelFormat.colorBits;
    pfd.cAlphaBits     = mPixelFormat.alphaBits;
    pfd.cDepthBits     = mPixelFormat.depthBits;
    pfd.cStencilBits   = mPixelFormat.stencilBits;
    pfd.iLayerType     = PFD_MAIN_PLANE;

    mPixelFormatIndex = ChoosePixelFormat( mDeviceContextHandle, &pfd );

    if(mPixelFormatIndex == 0) // Let's choose a default index.
    {
		mPixelFormatIndex = 1;    
		if(DescribePixelFormat( mDeviceContextHandle, mPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd ) == 0)
			return false;
    }

	mIsPixelFormatDescribed = true;

	return true;
}

bool WindowsOpenGLContext::setAdvancedPixelFormat()
{
	int valid = 0;
	unsigned int  numFormats = 0;
	float fAttributes[] = {0,0};
	int iFormatAttributes[22];
	
	iFormatAttributes[0] = WGL_DRAW_TO_WINDOW_ARB; iFormatAttributes[1] = GL_TRUE;
	iFormatAttributes[2] = WGL_SUPPORT_OPENGL_ARB; iFormatAttributes[3] = GL_TRUE;
	iFormatAttributes[4] = WGL_ACCELERATION_ARB;   iFormatAttributes[5] = WGL_FULL_ACCELERATION_ARB;
	iFormatAttributes[6] = WGL_COLOR_BITS_ARB;     iFormatAttributes[7] = mPixelFormat.colorBits;
	
	iFormatAttributes[ 8] = WGL_ALPHA_BITS_ARB;     iFormatAttributes[ 9] = mPixelFormat.alphaBits;
	iFormatAttributes[10] = WGL_DEPTH_BITS_ARB;     iFormatAttributes[11] = mPixelFormat.depthBits;
	iFormatAttributes[12] = WGL_STENCIL_BITS_ARB;   iFormatAttributes[13] = mPixelFormat.stencilBits;
	iFormatAttributes[14] = WGL_DOUBLE_BUFFER_ARB;  iFormatAttributes[15] = mPixelFormat.doubleBuffer;
	
	iFormatAttributes[16] = WGL_SAMPLE_BUFFERS_ARB; iFormatAttributes[17] = mPixelFormat.samples ? GL_TRUE : GL_FALSE;
	iFormatAttributes[18] = WGL_SAMPLES_ARB;        iFormatAttributes[19] = mPixelFormat.samples;
	iFormatAttributes[20] = 0;                      iFormatAttributes[21] = 0;
	
	if(GLEE_WGL_ARB_pixel_format)
	{
		valid = wglChoosePixelFormatARB(mDeviceContextHandle, iFormatAttributes, fAttributes, 1, &mPixelFormatIndex, &numFormats);
	}
	
	if (!(valid && numFormats >= 1))
	{
		Log::Instance().warning("WindowsOpenGLContext::setAdvancedPixelFormat", "Can't coose advanced pixel format");
		return false;
	}

	return true;
}

}//namespace GL {
}//namespace Squirrel {