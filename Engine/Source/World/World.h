#pragma once

#include "SceneBase.h"
#include "SceneNode.h"
#include "Terrain.h"
#include <Render/IRenderable.h>

namespace Squirrel {
namespace World { 

class SQWORLD_API Sky
{
public:
	virtual ~Sky(void) {}

	virtual void render(Render::IRender * render, Render::Camera * camera) = 0;
	virtual vec4 getHorizontColor() = 0;

};

class SQWORLD_API World: 
	public SceneObjectsContainer,
	public SceneNodesManager
{
	Terrain * mTerrain;
	Sky * mSky;
	bool mOwnsSky;

	float mUnitsInMeter;
	float mNodeSize;
	
	static const int MAX_NODES_NUM = 41;
	
	std::auto_ptr<SceneNode> mSceneNodes[MAX_NODES_NUM][MAX_NODES_NUM][MAX_NODES_NUM];
	tuple3i mSceneNodesNum;
	vec3 mSceneNodeSize;
	
	tuple3i mCenterNodePos;

	AABB mBounds;

	bool mSaveNewNodes;
	bool mCreateMissingNodes;

	std::auto_ptr<FileSystem::FileStorage> mContentSource;
	
	SCENE_OBJECTS_LIST mOrphans;
	
private:

	tuple3i getNextNodePos(vec3 beholderPos);
	SceneNode * loadNode(tuple3i gridPos);
	vec3 getOffsetForNodeIndex(int x, int y, int z);
	vec3 getGlobalOffsetForNodePos(int x, int y, int z);
	std::string createNodeFileName(tuple3i nodePos);

	template<class _Pred>
	SceneNode * traverseNodes(_Pred pred)
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
						if(pred(node))
						{
							return node;
						}
					}
				}
			}
		}

		return NULL;
	}
	
public:
	World(void);
	virtual ~World(void);

	void reset();
	void init(Settings * settings);

	void setCenter(tuple3i newCenterNodePos);
	
	Math::AABB getVisibleBounds();
	
	virtual void addSceneObject(SceneObject * sceneObj);
	virtual bool delSceneObject(SCENE_OBJECTS_LIST::const_iterator it);
	
	float getEffectiveViewDistance();

	float getUnitsInMeter() { return mUnitsInMeter; }
	void setUnitsInMeter(float u) { mUnitsInMeter = u; }

	Terrain * getTerrain() { return mTerrain; }
	void setTerrain(Terrain * terra) { mTerrain = terra; }

	Sky * getSky() { return mSky; }
	void setSky(Sky * sky, bool own = true) { mSky = sky; mOwnsSky = own; }

	void renderRecursively(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);
	void updateRecursively(float dtime);
	void updateTransform();

	bool load(Data * data);
	bool save(Data * data);

	void saveUnsavedNodes();
	
protected:
	
	void adoptObject(SCENE_OBJECTS_LIST::iterator itAdopt, SceneNode * prevOwner);
	SceneNode * findNewParent(SceneObject * obj);
};


}//namespace World { 

}//namespace Squirrel {

