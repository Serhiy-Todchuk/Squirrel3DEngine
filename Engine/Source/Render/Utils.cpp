#include "Utils.h"
#include "IRender.h"
#include "Camera.h"

namespace Squirrel {
namespace Render {

using namespace Math;

void Utils::Begin2D()
{
	IRender * render = IRender::GetActive();

	render->setViewport(0, 0, render->getWindow()->getSize().x, render->getWindow()->getSize().y);
	static Camera * cam2d = NULL;
	if(cam2d == NULL)
	{
		cam2d = new Camera(Camera::Orthographic);
	}
	cam2d->buildProjection(0.0f,	(float)render->getWindow()->getSize().x, 
									(float)render->getWindow()->getSize().y, 
									0.0f, -100.0f, 100.0f);
	cam2d->update();

	mat4 projectionMatrix = cam2d->getProjMatrix();
	render->setProjection(projectionMatrix);
	render->setTransform(mat4().identity());

	render->setAlphaTestValue(0.01f);
	render->setBlendMode(Render::IRender::blendOneMinusAlpha);
}

void Utils::End2D()
{
}

}//namespace Render { 
}//namespace Squirrel {

