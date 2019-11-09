#pragma once

#include "Renderable.h"
#include <Math/vec3.h>
#include <Math/Ray.h>
#include <Reflection/Object.h>
#include <list>

namespace Squirrel {

namespace Render {
	class IRender;
}//namespace Render {

using namespace Math;

namespace World { 

class SceneObject;

enum RaycastPrecision
{
	raycastBoundsOnly,
	raycastFast,
	raycastClosest
};

struct RaycastHit
{
	SceneObject * obj;
	vec3 position;
	vec3 normal;
	vec3 baricentricCoords;
	float distance;
	int triangleIndex;
};

typedef std::list<RaycastHit> RAYCASTHITS_LIST;

class SQWORLD_API SceneObjectsContainer: 
	public Reflection::Object
{
public:

	typedef std::list<SceneObject *> SCENE_OBJECTS_LIST;

public:
	SceneObjectsContainer(void);
	virtual ~SceneObjectsContainer(void);

	virtual bool findAllIntersections(Ray ray, RAYCASTHITS_LIST& outList);
	virtual bool findClosestIntersection(Ray ray, RaycastHit& out);

	virtual void addSceneObject(SceneObject * sceneObj);
	virtual bool delSceneObject(SCENE_OBJECTS_LIST::const_iterator it);
	virtual bool moveSceneObject(SCENE_OBJECTS_LIST::const_iterator it, SceneObjectsContainer * dstParent);
	const SCENE_OBJECTS_LIST& getSceneObjects() const { return mSceneObjects; }
	SCENE_OBJECTS_LIST::const_iterator findSceneObjectIt(SceneObject * sceneObj);
	void clearSceneObjects();

	SceneObject * findChildWithName(const std::string& name);

	virtual void updateRecursively(float dtime);

	void checkVisibility(Render::Camera * camera, SceneObjectsContainer * dst);

	//Renderable
	virtual void renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);
	virtual void renderCustomRecursively(Render::IRender * render, Render::Camera * camera, const RenderInfo& info);
	virtual void renderDebugInfoRecursively(Render::IRender * render, Render::Camera * camera);

protected:

	SCENE_OBJECTS_LIST mSceneObjects;

	bool mObjectsOwner;
};

}//namespace World { 

}//namespace Squirrel {

