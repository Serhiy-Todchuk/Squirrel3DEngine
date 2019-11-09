#pragma once

#include <Common/Window.h>

# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN   1          // Exclude rarely-used stuff from Windows headers
# endif
#include <windows.h>
#include "../OpenGL.h"

namespace Squirrel {
namespace GL {

class SQOPENGL_API WindowsOpenGLContext:
	public Context
{
private:
	WindowsOpenGLContext(const PixelFormat& pf);

public:
	WindowsOpenGLContext(const PixelFormat& pf, Window * window);
	virtual ~WindowsOpenGLContext();

	virtual void		activate();
	virtual void		deactivate();
	virtual Context *	createSibling()	const;
	virtual void		setSwapInterval(int interval);
	virtual void		finishFrame();

	virtual bool		isOk() const { return mRenderingContextHandle != NULL; }

	HDC getDeviceContextHandle() const { return mDeviceContextHandle; }

private:

	virtual bool initCaps();

	bool setPixelFormat();
	bool setAdvancedPixelFormat();

	HGLRC	mRenderingContextHandle;
	HDC		mDeviceContextHandle;

	int						mPixelFormatIndex;
	PIXELFORMATDESCRIPTOR	mPixelFormatDescription;
	bool					mIsPixelFormatDescribed;
};

}//namespace GL {
}//namespace Squirrel {