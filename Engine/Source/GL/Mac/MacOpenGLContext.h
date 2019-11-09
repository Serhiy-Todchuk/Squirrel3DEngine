#pragma once

#include <Common/Window.h>

#include "../OpenGL.h"

namespace Squirrel {
namespace GL {

class SQOPENGL_API MacOpenGLContext:
	public Context
{
private:
	MacOpenGLContext(const PixelFormat& pf);

public:
	MacOpenGLContext(const PixelFormat& pf, Window * window);
	virtual ~MacOpenGLContext();

	virtual void		activate();
	virtual void		deactivate();
	virtual Context *	createSibling()	const;
	virtual void		setSwapInterval(int interval);
	virtual void		finishFrame();

private:

	virtual bool initCaps();
	
	void *mNSOpenGLContext;
	void *mNSOpenGLPixelFormat;
};

}//namespace GL {
}//namespace Squirrel {