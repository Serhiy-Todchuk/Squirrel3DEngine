#pragma once

#include "Image.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ITexture.h"
#include "IProgram.h"
#include "IFrameBuffer.h"
#include "Camera.h"
#include "IRender.h"
#include "RenderQueue.h"

namespace Squirrel {

namespace Render { 

class IRenderable
{
public:
	IRenderable() {}
	virtual ~IRenderable() {}

	virtual void render(Render::IRender * render, Render::Camera * camera)						{}
	virtual void render(Render::RenderQueue * renderQueue, Render::Camera * camera)		{}
	virtual void renderDebugInfo(Render::IRender * render, Render::Camera * camera, int flags)	{}
	virtual bool isInCamera(Render::Camera * camera)							{ return false; }

};

}//namespace Render { 

}//namespace Squirrel {

