#pragma once

#include "SceneBase.h"
#include "macros.h"

namespace Squirrel {

namespace Render {
	class IRender;
}//namespace Render {

using namespace Math;

namespace World { 

class SceneNode;
	
class SQWORLD_API SceneNodesManager
{
public:
	SceneNodesManager() {}
	virtual ~SceneNodesManager() {}

public:
	virtual void adoptObject(SceneObjectsContainer::SCENE_OBJECTS_LIST::iterator itAdopt, SceneNode * prevOwner) = 0;
};
	
class SQWORLD_API SceneNode:
	public SceneObjectsContainer,
	public SceneNodesManager
{
public:
	SceneNode(void);
	virtual ~SceneNode(void);

	virtual void addSceneObject(SceneObject * child);
	virtual bool delSceneObject(SCENE_OBJECTS_LIST::const_iterator it);
	virtual bool moveSceneObject(SCENE_OBJECTS_LIST::const_iterator it, SceneObjectsContainer * dstParent);
	
	void updateTransform();
	
	void setOffset(const vec3& offset);
	inline vec3 getOffset() const { return mOffset; }
	
	inline void setSize(const vec3& size) { mSize = size; updateStaticBounds(); }
	
	inline const AABB& getStaticBounds() const { return mStaticBounds; }
	inline const AABB& getDynamicBounds() const { return mDynamicBounds; }
	
	inline void setGridPos(tuple3i pos) { mGridPos = pos; }
	inline tuple3i getGridPos() const { return mGridPos; }
	
	inline void setMaster(SceneNodesManager * master) { mMaster = master; }
	
	bool load(Data * data);
	bool save(Data * data);
	Data * save();
	
	static SceneNode * Load(Data * data);
	
protected:
	
	void updateStaticBounds();
	
	virtual void adoptObject(SCENE_OBJECTS_LIST::iterator itAdopt, SceneNode * prevOwner);
	
protected:
	
	tuple3i		mGridPos;
	
	vec3 mOffset;
	vec3 mSize;
	
	AABB mStaticBounds;
	AABB mDynamicBounds;
	
	bool mNeedToRecalculateBounds;
	
	SceneNodesManager * mMaster;
};

}//namespace World { 

}//namespace Squirrel {

