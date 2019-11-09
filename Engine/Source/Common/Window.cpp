#include "Window.h"

namespace Squirrel {

Window::Window(): 
	mPosition(0, 0), mSize(1, 1), mWindowMode(wmStandard), 
	mIsFullscreen(false), mCurrentContext(NULL), mMainWindow(false)
{
}

Window::~Window()
{
}

void Window::registerContext(Context * ctx)
{ 
	mRenderingContexts.push_back(ctx); 
	if(mCurrentContext == NULL)
		setCurrentContext(ctx);
}

void Window::setCurrentContext(Context * ctx)
{
	if(mCurrentContext != NULL)
	{
		mCurrentContext->deactivate();
	}
	if(ctx != NULL)
	{
		ctx->activate();
	}
	mCurrentContext = ctx;
}

}//namespace Squirrel {
