#include <GL/Render.h>

#ifdef _WIN32
#	include <GL/Windows/WindowsOpenGLContext.h>
#elif defined(__APPLE__)
#	include <GL/Mac/MacOpenGLContext.h>
#endif

extern "C" SQOPENGL_API Squirrel::Render::IRender * CreateRender()
{
	return new Squirrel::GL::Render();
}

extern "C" SQOPENGL_API void DestroyRender(Squirrel::Render::IRender * render)
{
	if(render)
		delete render;
}

extern "C" SQOPENGL_API Squirrel::Context * CreateContext(const Squirrel::PixelFormat& pf, Squirrel::Window * window)
{
#ifdef _WIN32
	return new Squirrel::GL::WindowsOpenGLContext(pf, window);
#elif defined(__APPLE__)
	return new Squirrel::GL::MacOpenGLContext(pf, window);
#endif
}

extern "C" SQOPENGL_API void DestroyContext(Squirrel::Context * render)
{
	if(render)
		delete render;
}

extern "C" SQOPENGL_API const char_t * GetRenderName()
{
	return "OpenGL 3.x renderer";
}
