#include "SceneBase.h"
#include "Body.h"
#include <Render/IRender.h>
#include <Reflection/CollectionWrapper.h>
#include <Reflection/XMLSerializer.h>
#include <Reflection/XMLDeserializer.h>

namespace Squirrel {

namespace World { 

SceneObjectsContainer::SceneObjectsContainer():
	mObjectsOwner(false)
{	
	SQREFL_SET_CLASS(World::SceneObjectsContainer);

	wrapCollectionField<SCENE_OBJECTS_LIST, SceneObject>("SceneObjects", &mSceneObjects);
}

SceneObjectsContainer::~SceneObjectsContainer(void)
{
	clearSceneObjects();
}

void SceneObjectsContainer::clearSceneObjects()
{
	if(mObjectsOwner)
	{
		SCENE_OBJECTS_LIST::iterator itChild = mSceneObjects.begin();
		while(itChild != mSceneObjects.end())
		{
			DELETE_PTR( (*itChild) );
			++itChild;
		}
	}
	mSceneObjects.clear();
}

void SceneObjectsContainer::addSceneObject(SceneObject * sceneObj)
{ 
	mSceneObjects.push_back(sceneObj); 
}

bool SceneObjectsContainer::moveSceneObject(SCENE_OBJECTS_LIST::const_iterator it, SceneObjectsContainer * dstParent)
{
	if(it != mSceneObjects.end())
	{
		SceneObject * obj = const_cast<SceneObject *>(*it);
		mSceneObjects.erase(it);
		dstParent->addSceneObject(obj);
		return true;
	}
	return false;
}


bool SceneObjectsContainer::delSceneObject(SCENE_OBJECTS_LIST::const_iterator it)
{
	if(it != mSceneObjects.end())
	{
		if(mObjectsOwner)
		{
			SceneObject * obj = const_cast<SceneObject *>(*it);
			DELETE_PTR( obj );
		}
		mSceneObjects.erase(it);
		return true;
	}
	return false;
}

SceneObjectsContainer::SCENE_OBJECTS_LIST::const_iterator SceneObjectsContainer::findSceneObjectIt(SceneObject * sceneObj)
{
	for(SCENE_OBJECTS_LIST::const_iterator it = mSceneObjects.cbegin(); it != mSceneObjects.cend(); ++it)
	{
		if((*it) == sceneObj)
		{
			return it;
		}
	}
	return mSceneObjects.cend();
}

bool SceneObjectsContainer::findAllIntersections(Ray ray, RAYCASTHITS_LIST& outList)
{
	vec3 normal, point;

	bool result = false;

	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		if((*it)->findAllIntersections(ray, outList))
		{
			result = true;
		}
	}	

	return result;
}

bool SceneObjectsContainer::findClosestIntersection(Ray ray, RaycastHit& out)
{
	RAYCASTHITS_LIST hitsList;

	if(!findAllIntersections(ray, hitsList)) return false;

	RaycastHit& closestHit = hitsList.front();
	float closestDistance = (ray.mOrigin - closestHit.position).lenSquared();
	for(RAYCASTHITS_LIST::iterator it = hitsList.begin(); it != hitsList.end(); ++it)
	{
		float dst = (ray.mOrigin - (*it).position).lenSquared();
		if(dst < closestDistance)
		{
			closestHit = (*it);
			closestDistance = dst;
		}
	}

	out = closestHit;

	return true;
}

void SceneObjectsContainer::checkVisibility(Render::Camera * camera, SceneObjectsContainer * dst)
{
	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		(*it)->mVisible = (*it)->isInCamera(camera);

		if((*it)->mVisible && dst != NULL)
			dst->addSceneObject((*it));
	}	
}

void SceneObjectsContainer::updateRecursively(float dtime)
{	
	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		(*it)->updateRecursively(dtime);
	}	
}

void SceneObjectsContainer::renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		(*it)->renderRecursively(renderQueue, camera, info);
	}
}

void SceneObjectsContainer::renderCustomRecursively(Render::IRender * render, Render::Camera * camera, const RenderInfo& info)
{
	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		(*it)->renderCustomRecursively(render, camera, info);
	}
}

void SceneObjectsContainer::renderDebugInfoRecursively(Render::IRender * render, Render::Camera * camera)
{
	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		(*it)->renderDebugInfoRecursively(render, camera);
	}
}

SceneObject * SceneObjectsContainer::findChildWithName(const std::string& name)
{
	for(SCENE_OBJECTS_LIST::iterator it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		if((*it)->getName() == name)
		{
			return (*it);
		}
		else
		{
			SceneObject * obj = (*it)->findChildWithName(name);
			if(obj != NULL)
			{
				return obj;
			}
		}
	}
	return NULL;
}

}//namespace World { 

}//namespace Squirrel {
