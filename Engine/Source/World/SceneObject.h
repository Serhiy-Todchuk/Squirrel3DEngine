#pragma once

#include <Resource/Animation.h>
#include <Resource/Animatable.h>
#include <Render/IRenderable.h>
#include "Behaviour.h"
#include "SceneBase.h"
#include "macros.h"
#include <list>
#include <memory>

namespace Squirrel {
namespace World { 

class SceneNode;

using namespace Resource;
	
class SQWORLD_API SceneObject: 
	public SceneObjectsContainer, 
	public Animatable
{
public:

	typedef std::list<Behaviour *> BEHAVIOUR_LIST;

	SceneObject(void);
	virtual ~SceneObject(void);

	bool updateTransform();

	virtual void addSceneObject(SceneObject * child);
	virtual bool delSceneObject(SCENE_OBJECTS_LIST::const_iterator it);
	virtual bool moveSceneObject(SCENE_OBJECTS_LIST::const_iterator it, SceneObjectsContainer * dstParent);

	template <class _TBeh>
	_TBeh * addBehaviour()
	{
		_TBeh * beh = new _TBeh;
		addBehaviour(beh);
		return beh;
	}
	void addBehaviour(Behaviour * behaviour);

	virtual bool isInCamera(Render::Camera * camera);

	virtual void saveSubAnims() {}

	//accessors

	bool	isEnabled() { return mEnabled; }
	bool	isGlobal() { return mGlobal; }

	const std::string&	getName	()	const		{return mName;}

	AABB	getAABB()			{ return mAABB; }
	AABB	getAllAABB()		{ return mAllAABB; }

	vec3	getPosition();
	vec3	getScale();
	quat	getRotation();

	vec3	getLocalPosition();
	vec3	getLocalScale();
	quat	getLocalRotation();

	mat4&		getLocalTransform()			{ return mLocalTransform; }
	const mat4&	getTransform()		const	{ return mTransform; }
	const mat4&	getLocalTransform()	const	{ return mLocalTransform; }

	BEHAVIOUR_LIST *	getBehaviours()	{ return &mBehaviours; }
	AnimationRunner *	getAnimations()	{ return mAnimations.get(); }

	SceneObject *		getParent() const	{ return mParent; }
	SceneNode *			getParentNode()const{ return mParentNode; }

	void	setGlobal(bool global) { mGlobal = global; }

	void	setName	(const std::string& name)	{mName	= name;}

	void	setLocalPosition(vec3 pos);
	void	setLocalRotation(quat rot);
	void	setLocalScale(vec3 scale);
	void	setLocalTransform(mat4 tform);
	
	void	setPosition(vec3 pos);

	virtual void deserialize(Reflection::Deserializer * deserializer);
	virtual void serialize(Reflection::Serializer * serializer);

protected:

	void extractLocalTransforms();

	void invalidateChildren();
	void initMembers();

	virtual void calcAABB();

	virtual void update(float dtime) {}

	virtual void render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info) {}

	virtual void renderCustom(Render::IRender * render, Render::Camera * camera, const RenderInfo& info)	{}

private:

	friend class SceneObjectsContainer;
	friend class SceneNode;

	virtual void renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);
	virtual void renderCustomRecursively(Render::IRender * render, Render::Camera * camera, const RenderInfo& info);
	
	void updateRecursively(float dtime);
	
private:
	
	Math::AABB		  mAllAABB;

protected:

	//content members

	std::auto_ptr<AnimationRunner> mAnimations;
	BEHAVIOUR_LIST	  mBehaviours;
	Math::AABB		  mAABB;

	//state members

	bool				mEnabled;
	bool				mVisible;

	bool				mGlobal;

	//transform members

	bool				mPositionChanged;
	bool				mRotationChanged;
	bool				mScaleChanged;
	bool				mTransformChanged;
	bool				mExtractLocals;

	vec3				mLocalPosition;
	vec3				mLocalScale;
	quat				mLocalRotation;

	mat3				mLocalRotationMatrix;

	vec3				mPosition;
	vec3				mScale;
	quat				mRotation;

	mat4				mTransform;
	mat4				mLocalTransform;

	//hierarchy members

	std::string			mName;

	SceneObject *		mParent;
	SceneObject *		mMaster;

	SceneNode *			mParentNode;
};

inline void SceneObject::setLocalTransform(mat4 tform)
{
	mLocalTransform		= tform;
	mTransformChanged	= true;
	mExtractLocals		= true;
}

inline void SceneObject::setLocalRotation(quat rot)			
{ 
	mLocalRotation = rot; 
	mRotationChanged = true; 
}

inline void SceneObject::setLocalScale(vec3 scale)		
{ 
	mLocalScale = scale;  
	mScaleChanged = true; 
}

inline vec3	SceneObject::getLocalScale()		
{ 
	extractLocalTransforms();
	return mLocalScale; 
}

inline quat	SceneObject::getLocalRotation()		
{
	extractLocalTransforms();
	return mLocalRotation; 
}


}//namespace World { 
}//namespace Squirrel {

