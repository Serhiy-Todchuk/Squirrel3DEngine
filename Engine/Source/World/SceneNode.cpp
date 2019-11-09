#include "SceneNode.h"
#include "Body.h"
#include <Render/IRender.h>
#include <Reflection/CollectionWrapper.h>
#include <Reflection/BinSerializer.h>
#include <Reflection/BinDeserializer.h>

namespace Squirrel {

namespace World { 

SceneNode::SceneNode(): mOffset(0, 0, 0), mMaster(NULL), mSize(0, 0, 0)
{
	mObjectsOwner = false;
	mNeedToRecalculateBounds = false;
}

SceneNode::~SceneNode(void)
{
}
	
SceneNode * SceneNode::Load(Data * data)
{
	SceneNode * node = new SceneNode;
	
	if(!node->load(data))
	{
		DELETE_PTR(node);
		return NULL;
	}
	
	return node;
}
	
bool SceneNode::load(Data * data)
{
	Reflection::BinDeserializer deserializer;
	deserializer.loadFrom(data->getData(), data->getLength(), false);
	
	deserialize(&deserializer);

	SCENE_OBJECTS_LIST::iterator it;

	for(it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		SceneObject * obj = (*it);

		obj->mParentNode = this;
	}
	
	return true;
}
	
bool SceneNode::save(Data * data)
{
	Reflection::BinSerializer serializer;
	
	serialize(&serializer);
	
	Reflection::DATA_PTR xmlData = serializer.getData();
	data->putData(xmlData->data, xmlData->length);
	
	return true;
}

Data * SceneNode::save()
{
	Data * data = new Data(NULL, 0);

	if(save(data))
		return data;

	delete data;
	return NULL;
}

void SceneNode::updateTransform()
{
	mDynamicBounds.reset();
	
	std::list<SCENE_OBJECTS_LIST::iterator> objsToAdopt;
	SCENE_OBJECTS_LIST::iterator it;
	
	for(it = mSceneObjects.begin(); it != mSceneObjects.end(); ++it)
	{
		SceneObject * obj = (*it);
		
		obj->updateTransform();
		
		if(!mStaticBounds.intersects(obj->getAllAABB()) || obj->isGlobal())
		{
			objsToAdopt.push_back(it);
		}
		else
		{
			mDynamicBounds.merge(obj->getAllAABB());
		}
	}
	
	for(std::list<SCENE_OBJECTS_LIST::iterator>::iterator itAdopt = objsToAdopt.begin();
		itAdopt != objsToAdopt.end(); ++itAdopt)
	{
		it = *itAdopt;
		
		if(mMaster)
		{
			mMaster->adoptObject(it, this);
		}
	}
}
	
void SceneNode::updateStaticBounds()
{
	vec3 halfSize = mSize * 0.5f;
	mStaticBounds.min = mOffset - halfSize;
	mStaticBounds.max = mOffset + halfSize;
}
	
void SceneNode::setOffset(const vec3& offset)
{
	mOffset = offset;
	
	updateStaticBounds();

	//updateTransform();
}

void SceneNode::adoptObject(SCENE_OBJECTS_LIST::iterator itAdopt, SceneNode * prevOwner)
{
	ASSERT(false);
}

void SceneNode::addSceneObject(SceneObject * child)
{
	ASSERT( child != NULL );
	
	mDynamicBounds.merge(child->getAllAABB());

	child->mParentNode = this;
	
	mSceneObjects.push_back(child);
}
	
bool SceneNode::delSceneObject(SCENE_OBJECTS_LIST::const_iterator it)
{
	SceneObject * obj = (*it);

	bool result = SceneObjectsContainer::delSceneObject(it);
	
	if(result)
	{
		mNeedToRecalculateBounds = true;
		obj->mParentNode = NULL;
	}
	
	return result;
}
	
bool SceneNode::moveSceneObject(SCENE_OBJECTS_LIST::const_iterator it, SceneObjectsContainer * dstParent)
{
	SceneObject * obj = (*it);

	bool result = SceneObjectsContainer::moveSceneObject(it, dstParent);
	
	if(result)
	{
		mNeedToRecalculateBounds = true;
		if(obj->mParentNode == this)
			obj->mParentNode = NULL;
	}
	
	return result;
}


}//namespace World { 

}//namespace Squirrel {
