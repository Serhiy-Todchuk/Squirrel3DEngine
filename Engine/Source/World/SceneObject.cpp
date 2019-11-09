#include "SceneObject.h"
#include "SceneNode.h"
#include <Resource/TextureStorage.h>
#include <Resource/ModelStorage.h>
#include <Resource/AnimationRunner.h>
#include <Reflection/AtomicWrapper.h>
#include <Reflection/CollectionWrapper.h>
#include <Reflection/EnumWrapper.h>

namespace Squirrel {

namespace World { 

SQREFL_REGISTER_CLASS_SEED(World::SceneObject, WorldSceneObject);

SceneObject::SceneObject()
{
	mObjectsOwner = true;

	initMembers();

	Field * field = NULL;

	SQREFL_SET_CLASS(World::SceneObject);

	wrapAtomicField("Name", &mName);

	wrapAtomicField("Enabled", &mEnabled);

	field = wrapAtomicField("Position", &mLocalPosition.x, 3);
	field->changeFlag = &mPositionChanged;
	field = wrapAtomicField("Rotation", &mLocalRotation.x, 4);
	field->changeFlag = &mRotationChanged;
	field = wrapAtomicField("Scale",	&mLocalScale.x, 3);
	field->changeFlag = &mScaleChanged;

	wrapCollectionField<BEHAVIOUR_LIST, Behaviour>("Behaviours", &mBehaviours);
}

SceneObject::~SceneObject(void)
{
	for(BEHAVIOUR_LIST::iterator it = mBehaviours.begin(); it != mBehaviours.end(); ++it)
	{
		DELETE_PTR( (*it) );
	}
}

void SceneObject::initMembers()
{
	mEnabled = true;
	mVisible = false;
	mGlobal  = false;

	//transform members

	mTransformChanged	= false;

	mPositionChanged	= false;
	mRotationChanged	= false;
	mScaleChanged		= false;
	mExtractLocals		= false;

	mPosition			= vec3(0,0,0);
	mScale				= vec3(1,1,1);
	//mRotation;
	mLocalPosition		= vec3(0,0,0);
	mLocalScale			= vec3(1,1,1);
	//mLocalRotation;

	mTransform.identity();
	mLocalTransform.identity();
	mLocalRotationMatrix.identity();

	//hierarchy members

	mAABB.reset();

	mParent				= NULL;
	mMaster				= NULL;

	mParentNode			= NULL;
}

void SceneObject::deserialize(Reflection::Deserializer * deserializer)
{
	Object::deserialize(deserializer);

	SCENE_OBJECTS_LIST::iterator itChild = mSceneObjects.begin();
	while(itChild != mSceneObjects.end())
	{
		(*itChild)->mParent = this;
		++itChild;
	}
}

void SceneObject::serialize(Reflection::Serializer * serializer)
{
	mExtractLocals = true;
	extractLocalTransforms();

	Object::serialize(serializer);
}

void SceneObject::addBehaviour(Behaviour * behaviour)
{
	mBehaviours.push_back(behaviour); 
	behaviour->mSceneObject = this;
	behaviour->awake();
}

void SceneObject::updateRecursively(float dtime)
{
	if(mEnabled)
	{
		for(BEHAVIOUR_LIST::iterator it = mBehaviours.begin(); it != mBehaviours.end(); ++it)
		{
			Behaviour * behaviour = (*it);
			if(behaviour->isEnabled())
			{
				if(!behaviour->mHasStarted)
				{
					behaviour->start();
					behaviour->mHasStarted = true;
				}
				behaviour->update();
			}
		}
		if(mAnimations.get() != NULL)
		{
			mAnimations->update(dtime);
		}

		update(dtime);
	}

	SceneObjectsContainer::updateRecursively(dtime);
}

void SceneObject::setLocalPosition(vec3 pos)
{
	mLocalPosition = pos;
	mPositionChanged = true;
}

vec3	SceneObject::getLocalPosition()
{
	extractLocalTransforms();
	return mLocalPosition;
}
	
void SceneObject::setPosition(vec3 pos)
{
	if(mParent)
	{
		// position is relative to parent so transform upwards
		
		pos = mParent->getRotation().conj().apply( pos - mParent->getPosition() ).div( mParent->getScale() );
	}
	mLocalPosition = pos;
	mPositionChanged = true;
}

vec3	SceneObject::getPosition()		
{
	mPosition				= mLocalPosition;
	if(mParent)
	{
        // Change position vector based on parent's orientation & scale
		mPosition = mParent->getRotation().apply( mParent->getScale().mul( mPosition ) );

        // Add altered position vector to parents
		mPosition += mParent->getPosition();
	}
	return mPosition;
}

vec3	SceneObject::getScale()			
{ 
	mScale					= mLocalScale;
	if(mParent) mScale		= mParent->getScale().mul(mScale);
	return mScale; 
}

quat	SceneObject::getRotation()			
{ 
	mRotation					= mLocalRotation;
	if(mParent)	mRotation		= mParent->getRotation() & mRotation;
	return mRotation; 
}

void SceneObject::extractLocalTransforms()
{
	if(mExtractLocals)
	{
		mLocalPosition			= mLocalTransform.getTranslate();
		mLocalRotationMatrix	= mLocalTransform.getMat3();
		mLocalRotationMatrix.orthonormalize();
		mLocalRotation			= quat().fromRotationMatrix( mLocalRotationMatrix );
		mLocalScale				= mLocalTransform.extractScale();

		mExtractLocals			= false;
	}
}

bool SceneObject::updateTransform()
{
	//update node transform

	if(mRotationChanged)
	{
		mLocalRotationMatrix = mLocalRotation.toRotationMatrix();
	}

	bool needToUpdateLocalTransform = mRotationChanged || mPositionChanged || mScaleChanged;

	if(needToUpdateLocalTransform)
	{
		mLocalTransform = mat4::Transform(mLocalPosition, mLocalRotationMatrix, mLocalScale);
	}

	bool needToUpdateTransform = needToUpdateLocalTransform || mTransformChanged;

	if(needToUpdateTransform)
	{
		mTransform = mLocalTransform;
		if(mParent)
		{
			mTransform = mParent->mTransform * mTransform;
		}
		invalidateChildren();
	}

	//update bounding volume
	calcAABB();

	bool childTransformChanged = false;

	//update children as well
	SCENE_OBJECTS_LIST::iterator itChild = mSceneObjects.begin();
	while(itChild != mSceneObjects.end())
	{
		if((*itChild)->updateTransform()) childTransformChanged = true;
		++itChild;
	}

	//backward recursion logic

	//if(childTransformChanged)
	{
		//update AllAABB by combining children AABBs
		mAllAABB = mAABB;
		itChild = mSceneObjects.begin();
		while(itChild != mSceneObjects.end())
		{
			mAllAABB.merge( (*itChild)->getAllAABB() );
			++itChild;
		}
	}

	//reset flags
	mPositionChanged	= false;
	mScaleChanged		= false;
	mRotationChanged	= false;
	mTransformChanged	= false;

	return needToUpdateTransform || childTransformChanged;
}

void SceneObject::invalidateChildren()
{
	//update children as well
	SCENE_OBJECTS_LIST::iterator itChild = mSceneObjects.begin();
	while(itChild != mSceneObjects.end())
	{
		(*itChild)->mTransformChanged = true;
		++itChild;
	}
}
	
void SceneObject::calcAABB()
{
	mAABB.setPoint(getPosition());
}

void SceneObject::addSceneObject(SceneObject * child)
{
	ASSERT( child != NULL );
	child->mParent		= this;

	child->mTransformChanged	= true;

	mSceneObjects.push_back(child);
}

bool SceneObject::delSceneObject(SCENE_OBJECTS_LIST::const_iterator it)
{
	if(it == mSceneObjects.end()) return false;
	if((*it)->mMaster == NULL)
	{
		return SceneObjectsContainer::delSceneObject(it);
	}
	return false;
}

bool SceneObject::moveSceneObject(SCENE_OBJECTS_LIST::const_iterator it, SceneObjectsContainer * dstParent)
{
	if(it == mSceneObjects.end()) return false;
	if((*it)->mMaster == NULL)
	{
		return SceneObjectsContainer::moveSceneObject(it, dstParent);
	}
	return false;
}

bool SceneObject::isInCamera(Render::Camera * camera)
{
	return camera->isAABBIn(mAABB);
}

void SceneObject::renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	bool visible = true;

	if(camera != NULL)
	{
		visible = isInCamera(camera);
	}

	if(visible && mEnabled)
	{
		render(renderQueue, camera, info);
	}

	SceneObjectsContainer::renderRecursively(renderQueue, camera, info);
}

void SceneObject::renderCustomRecursively(Render::IRender * render, Render::Camera * camera, const RenderInfo& info)
{
	bool visible = true;

	if(camera != NULL)
	{
		visible = isInCamera(camera);
	}

	if(visible)
	{
		renderCustom(render, camera, info);
	}

	SceneObjectsContainer::renderCustomRecursively(render, camera, info);
}

}//namespace World { 

}//namespace Squirrel {
