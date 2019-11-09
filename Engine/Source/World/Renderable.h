#pragma once

#include "macros.h"
#include <Render/IRender.h>
#include <Render/RenderQueue.h>
#include <Render/Camera.h>

namespace Squirrel {

namespace Render {
	class IRender;
}//namespace Render {

using namespace Math;

namespace World { 

enum RenderInfoLevel
{
	rilStaticGeometry		= 1,
	rilSkinnedGeometry,
	rilDecalMaps,			//depth only level
	rilNormals,				//normals only level
	rilMaterials,
	rilLighting,
};

static const RenderInfoLevel sBuildShadowRILevel = RenderInfoLevel::rilDecalMaps;

#define sLODMaxDetails			1.0f
#define sLODGoodForReflections	3.0f

struct RenderInfo
{
	RenderInfo(): level(rilLighting), minLOD(0.0f), layersRange(0, Render::rqMax), clipPlane(NULL) {}

	RenderInfoLevel level;

	float minLOD;

	tuple2i layersRange;

	vec4 * clipPlane;//for reflections

	//put here allowed effects also
};

class SQWORLD_API RenderableContainer
{
public:
	virtual ~RenderableContainer() {}
	
	//Renderable
	virtual void renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info) = 0;
	virtual void renderCustomRecursively(Render::IRender * render, Render::Camera * camera, const RenderInfo& info) = 0;
	virtual void renderDebugInfoRecursively(Render::IRender * render, Render::Camera * camera) = 0;
};

class SQWORLD_API RenderableObject
{
public:
	virtual ~RenderableObject() {}

	//Renderable
	virtual void render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info) = 0;
	virtual void renderCustom(Render::IRender * render, Render::Camera * camera, const RenderInfo& info) = 0;
	virtual void renderDebugInfo(Render::IRender * render, Render::Camera * camera) = 0;
};

}//namespace World { 

}//namespace Squirrel {

