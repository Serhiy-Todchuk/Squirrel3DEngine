#include "World.h"
#include "SceneObject.h"
#include <Common/Settings.h>
#include <Reflection/CollectionWrapper.h>
#include <Reflection/XMLSerializer.h>
#include <Reflection/XMLDeserializer.h>
#include <FileSystem/Path.h>
#include <iomanip>

#define WORLD_SETTINGS_SECTION "World"

namespace Squirrel {
namespace World { 

World::World():
	mUnitsInMeter(1.0f), mTerrain(NULL), mSky(NULL), mOwnsSky(false), mCenterNodePos(0, 0, 0), mCreateMissingNodes(true), mSaveNewNodes(false)
{
	mObjectsOwner = true;

	SQREFL_SET_CLASS(World::World);

	wrapAtomicField("UnitsInMeter", &mUnitsInMeter);
	
	reset();
}

World::~World(void)
{
	if(mOwnsSky)
		DELETE_PTR(mSky);
}

void World::init(Settings * settings)
{
	std::string worldPath = settings->getString(WORLD_SETTINGS_SECTION, "Storage", "World");
	worldPath = FileSystem::Path::GetAbsPath(worldPath);

	FileSystem::FileStorage * contentSource = FileSystem::FileStorageFactory::GetFileStorageForPath( worldPath );
	if(contentSource != NULL)
	{
		mContentSource.reset( contentSource );
	}

	mUnitsInMeter = Settings::Default()->getFloat(WORLD_SETTINGS_SECTION, "UnitsInMeter", 1.0f);
	
	mSceneNodesNum.x = Settings::Default()->getInt(WORLD_SETTINGS_SECTION, "NodesNum", 17);
	mSceneNodesNum.y = 1;
	mSceneNodesNum.z = mSceneNodesNum.x;
	
	mSceneNodeSize.x = Settings::Default()->getFloat(WORLD_SETTINGS_SECTION, "NodeSize", 128.0f);
	mSceneNodeSize.y = Settings::Default()->getFloat(WORLD_SETTINGS_SECTION, "NodeHeight", 900000.0f);
	mSceneNodeSize.z = mSceneNodeSize.x;
	
	setCenter(tuple3i(0, 0, 0));
}
	
void World::updateTransform()
{
	int i, j, k;//indices
	
	for(i = 0; i < mSceneNodesNum.x; ++i)
	{
		for(j = 0; j < mSceneNodesNum.y; ++j)
		{
			for(k = 0; k < mSceneNodesNum.z; ++k)
			{
				SceneNode * node = mSceneNodes[i][j][k].get();
				if(node != NULL)
				{
					node->updateTransform();
				}
			}
		}
	}
	
	SCENE_OBJECTS_LIST::iterator itOrphan = mOrphans.begin();
	while(itOrphan != mOrphans.end())
	{
		(*itOrphan)->updateTransform();
		
		SceneNode * newParent = (*itOrphan)->isGlobal() ? NULL : findNewParent(*itOrphan);
		if(newParent)
		{
			newParent->addSceneObject(*itOrphan);
			itOrphan = mOrphans.erase(itOrphan);
		}
		else
		{
			++itOrphan;
		}
	}
}

tuple3i World::getNextNodePos(vec3 beholderPos)
{
	tuple3i nextNodePos = mCenterNodePos;

	tuple3i centerIndex = (mSceneNodesNum - 1) / 2;
	SceneNode * currNode = mSceneNodes[centerIndex.x][centerIndex.y][centerIndex.z].get();

	AABB aabb = currNode->getStaticBounds();
	if(beholderPos.x > aabb.max.x)
		++nextNodePos.x;
	else if(beholderPos.x < aabb.min.x)
		--nextNodePos.x;
	if(beholderPos.y > aabb.max.y)
		++nextNodePos.y;
	else if(beholderPos.y < aabb.min.y)
		--nextNodePos.y;
	if(beholderPos.z > aabb.max.z)
		++nextNodePos.z;
	else if(beholderPos.z < aabb.min.z)
		--nextNodePos.z;

	return nextNodePos;
}
	
void World::updateRecursively(float dtime)
{
	Render::Camera * camera = Render::Camera::GetMainCamera();

	if(mTerrain)
	{
		tuple2i newNodePos = mTerrain->getNextNodePos(camera->getPosition());
				
		if(newNodePos != mTerrain->getCenterNodePos())
		{
			mTerrain->setCenter(newNodePos);
		}
	}
	
	tuple3i newNodePos = getNextNodePos(camera->getPosition());

	if(newNodePos != mCenterNodePos)
	{
		setCenter(newNodePos);
	}

	SceneObjectsContainer::updateRecursively(dtime);
}

void World::renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info)
{
	SceneObjectsContainer::renderRecursively(renderQueue, camera, info);

	if(mTerrain)
		mTerrain->render(renderQueue, camera, info);
}

void World::reset()
{
	clearSceneObjects();

	mSky = NULL;

	mUnitsInMeter = Settings::Default()->getFloat("World", "UnitsInMeter", 1.0f);
}

bool World::load(Data * data)
{
	Reflection::XMLDeserializer deserializer;
	deserializer.loadFrom(data->getData(), data->getLength());

	deserialize(&deserializer);

	return true;
}

bool World::save(Data * data)
{
	Reflection::XMLSerializer serializer;

	serialize(&serializer);

	Reflection::DATA_PTR xmlData = serializer.getData();
	data->putData(xmlData->data, xmlData->length);

	return true;
}

Math::AABB World::getVisibleBounds()
{
	AABB bounds = mBounds;

	if(mTerrain)
	{
		bounds.merge( mTerrain->getBounds() );
	}

	return bounds;
}
	
float World::getEffectiveViewDistance()
{
	//TODO

	if(mTerrain)
	{
		return mTerrain->getNodeSize() * (mTerrain->getNodesNum() - 1) * 0.5f;
	}
	
	return 1000;
}
	
std::string World::createNodeFileName(tuple3i nodePos)
{
	std::stringstream fileNameStr;
	
	const char_t * negativeTag	= "n";
	const char_t * positiveTag	= "p";
	const char_t * extension	= ".sqsn";
	
	const ptrdiff_t width = 4;
	
	fileNameStr.fill('0');
	
	fileNameStr << std::setw(0) << ((nodePos.x >= 0) ? positiveTag : negativeTag);
	fileNameStr << std::setw(width) << std::right << Math::absValue(nodePos.x);
	fileNameStr << std::setw(0) << ((nodePos.y >= 0) ? positiveTag : negativeTag);
	fileNameStr << std::setw(width) << std::right << Math::absValue(nodePos.y);
	fileNameStr << std::setw(0) << ((nodePos.z >= 0) ? positiveTag : negativeTag);
	fileNameStr << std::setw(width) << std::right << Math::absValue(nodePos.z);
	
	fileNameStr << std::setw(0) << extension;
	
	return fileNameStr.str();
}	
	
SceneNode * World::loadNode(tuple3i gridPos)
{
	std::string fileName = createNodeFileName(gridPos);
	
	Data * data = NULL;
	
	if(mContentSource.get())
		data = mContentSource->getMappedFile(fileName);
	
	SceneNode * node = NULL;
	
	if(data == NULL)
	{
		if(mCreateMissingNodes)
		{
			node = new SceneNode();
		}
	}
	else
	{
		node = SceneNode::Load(data);
		delete data;

		if(node)
		{
			FOREACH(SCENE_OBJECTS_LIST::const_iterator, itObj, node->getSceneObjects())
			{
				mSceneObjects.push_back(*itObj);
			}
		}
	}

	if(node)
	{
		node->setSize(mSceneNodeSize);
		node->setGridPos(gridPos);
		node->setMaster(this);
	}
	
	return node;
}

vec3 World::getOffsetForNodeIndex(int x, int y, int z)
{
	tuple3i centerIndex = (mSceneNodesNum - 1) / 2;
	vec3 offset(x - centerIndex.x, y - centerIndex.y, z - centerIndex.z);
	offset  = offset.mul(mSceneNodeSize);
	return offset;
}
	
vec3 World::getGlobalOffsetForNodePos(int x, int y, int z)
{
	vec3 offset(x, y, z);
	offset  = offset.mul(mSceneNodeSize);
	return offset;
}
	
SceneNode * World::findNewParent(SceneObject * obj)
{
	int i, j, k;//indices
	
	for(i = 0; i < mSceneNodesNum.x; ++i)
	{
		for(j = 0; j < mSceneNodesNum.y; ++j)
		{
			for(k = 0; k < mSceneNodesNum.z; ++k)
			{
				SceneNode * node = mSceneNodes[i][j][k].get();
				if(node != NULL)
				{
					if(node->getStaticBounds().intersects(obj->getAllAABB()))
					{
						return node;
					}
				}
			}
		}
	}
	
	return NULL;
}
	
bool World::delSceneObject(SCENE_OBJECTS_LIST::const_iterator it)
{
	FOREACH(SCENE_OBJECTS_LIST::iterator, itOrphan, mOrphans)
	{
		if((*itOrphan) == (*it))
		{
			mOrphans.erase(itOrphan);
			break;
		}
	}
	if((*it)->getParentNode() != NULL)
	{
		(*it)->getParentNode()->delSceneObject( (*it)->getParentNode()->findSceneObjectIt(*it) );
	}
	return SceneObjectsContainer::delSceneObject(it);
}
	
void World::addSceneObject(SceneObject * sceneObj)
{
	SceneNode * newParent = sceneObj->isGlobal() ? NULL : findNewParent(sceneObj);
	
	if(newParent != NULL)
	{
		newParent->addSceneObject(sceneObj);
	}
	else
	{
		mOrphans.push_back(sceneObj);
	}
	
	mSceneObjects.push_back(sceneObj);
}
	
void World::adoptObject(SCENE_OBJECTS_LIST::iterator itAdopt, SceneNode * prevOwner)
{
	SceneNode * newParent = (*itAdopt)->isGlobal() ? NULL : findNewParent(*itAdopt);
	
	if(newParent != NULL)
	{
		if(prevOwner != NULL)
		{
			prevOwner->moveSceneObject(itAdopt, newParent);
		}
		else
		{
			newParent->addSceneObject(*itAdopt);
		}
	}
	else
	{
		mOrphans.push_back(*itAdopt);
		prevOwner->delSceneObject(itAdopt);
	}
}

void World::saveUnsavedNodes()
{
	int i, j, k;//indices

	for(i = 0; i < mSceneNodesNum.x; ++i)
	{
		for(j = 0; j < mSceneNodesNum.y; ++j)
		{
			for(k = 0; k < mSceneNodesNum.z; ++k)
			{
				SceneNode * node = mSceneNodes[i][j][k].get();
				if(node != NULL && node->getSceneObjects().size() > 0)
				{
					std::string fileName = createNodeFileName(node->getGridPos());
					if(!mContentSource->hasFile(fileName))
					{
						Data * data = node->save();
						if(data)
							mContentSource->putFile(data, fileName);
					}
				}
			}
		}
	}
}
	
void World::setCenter(tuple3i newCenterNodePos)
{
	mCenterNodePos = newCenterNodePos;

	saveUnsavedNodes();
	
	//store existing nodes
	
	typedef std::map<tuple3i, SceneNode *> NODES_MAP;
	NODES_MAP loadedNodes;
	
	int x, z, y;//gridPos
	int i, j, k;//indices
	
	for(i = 0; i < mSceneNodesNum.x; ++i)
	{
		for(j = 0; j < mSceneNodesNum.y; ++j)
		{
			for(k = 0; k < mSceneNodesNum.z; ++k)
			{
				SceneNode * node = mSceneNodes[i][j][k].release();
				if(node != NULL)
				{
					loadedNodes[tuple3i(node->getGridPos())] = node;
				}
			}
		}
	}
	
	//load new nodes or use existing ones
	
	tuple3i centerIndex = (mSceneNodesNum - 1) / 2;
	
	tuple3i startNodePos = newCenterNodePos - centerIndex;

	vec3 center = getGlobalOffsetForNodePos(newCenterNodePos.x, newCenterNodePos.y, newCenterNodePos.z);
	vec3 size = vec3(mSceneNodeSize.x * mSceneNodesNum.x, mSceneNodeSize.y * mSceneNodesNum.y, mSceneNodeSize.z * mSceneNodesNum.z);

	mBounds.setCenterSize(center, size);

	for(x = startNodePos.x, i = 0; i < mSceneNodesNum.x; ++x, ++i)
	{
		for(y = startNodePos.y, j = 0; j < mSceneNodesNum.y; ++y, ++j)
		{
			for(z = startNodePos.z, k = 0; k < mSceneNodesNum.z; ++z, ++k)
			{
			
				tuple3i nodePos(x, y, z);
				
				SceneNode * node = NULL;
				
				NODES_MAP::iterator itLoaded = loadedNodes.find(nodePos);
				if(itLoaded != loadedNodes.end())
				{
					node = itLoaded->second;
					loadedNodes.erase(itLoaded);
				}
			
				if(node == NULL)
				{
					node = loadNode(nodePos);
				}
				
				if(node != NULL)
				{
					vec3 offset = getGlobalOffsetForNodePos(x, y, z);
					node->setOffset( offset );
				}
				
				mSceneNodes[i][j][k].reset(node);
			}
		}
	}
	
	//remove unused nodes
	
	FOREACH(NODES_MAP::iterator, itNode, loadedNodes)
	{
		DELETE_PTR(itNode->second);
	}
}	
	
}//namespace World { 
}//namespace Squirrel {
