#pragma once

#include "TerrainNode.h"
#include <FileSystem/FileStorageFactory.h>
#include <Common/DataMap.h>
#include <Math/PerlinNoise.h>
#include <Common/Settings.h>
#include <Resource/Program.h>
#include <memory>
#include "Renderable.h"

namespace Squirrel {
namespace World { 

using namespace Math;
	
class SQWORLD_API HeightGenerator
{
public:
	
	HeightGenerator();
	~HeightGenerator();
	
	void initSrcHM(std::string srcHMfileName, vec3 hmSize);
	void initNoise(std::string srcHMfileName, vec3 noiseScale);
	
	void apply(HeightMap * hm, vec3 offset, vec3 scale);
	
	bool		mNoise;
	vec3		mNoiseScale;
	uint		mNoiseSeed;
	PerlinNoise	mPerlinNoise;
	
	vec3		mHMSize;
	vec3		mHMOffset;
	std::auto_ptr<RenderData::Image>	mSrcHM;
	
private:
	float fetchHM(float x, float z);
	float getHMHeight(int x, int z);
};
	
class SQWORLD_API Terrain
{
public:

	enum EDirection
	{
		dirCenter = 0,
		dirNorth,
		dirNorthEast,
		dirEast,
		dirSouthEast,
		dirSouth,
		dirSouthWest,
		dirWest,
		dirNorthWest,
		dirNum
	};

public://ctor/dtor
	
	Terrain();
	virtual ~Terrain();

public://methods

	float height(float x, float z);

	void initTextures(const char_t * texture1Name, const char_t * texture2Name = NULL, const char_t * texture3Name = NULL, const char_t * texture4Name = NULL);
	void init(Settings * settings, bool autoGenerate = false);
	void render(Render::RenderQueue * renderQueue, Render::Camera * camera, const RenderInfo& info);
	
	tuple2i getNextNodePos(vec3 beholderPos);
	
	void setCenter(tuple2i newCentralNodePos);

	TerrainNode * getNode(int i, int j) { return mNodes[i][j].get(); }

	void setAsMain();
	static Terrain * GetMain();

	int getNodesNum()			const { return mNodesNum; }
	float getNodeSize()			const { return mNodeSize; }
	float getCellSize()			const { return mNodeSize / mCellsPerNode; }
	size_t getCellsPerNode()	const { return mCellsPerNode; }
	tuple2i getCenterNodePos()	const { return mCenterNodePos; }

	AABB getBounds() 
	{ 
		AABB bounds;
		for(int i = 0; i < mNodesNum; ++i)
		{
			for(int j = 0; j < mNodesNum; ++j)
			{
				if(mNodes[i][j].get() != NULL)
				{
					bounds.merge( mNodes[i][j]->getTransformedAABB() );
				}
			}
		}
		return bounds;
	}
	
	void saveUnsavedNodes();

private://methods
	
	TerrainNode* getCenterNode() {
		int centerIndex = (mNodesNum - 1) / 2;
		return mNodes[centerIndex][centerIndex].get();
	}
	
	HeightMap * loadHM(tuple2i gridPos);
	TerrainNode * makeNode(HeightMap * hm, tuple2i gridPos, int lod);
	
	std::string createHMFileName(tuple2i gridPos);
	
	vec3 getOffsetForNodeIndex(int x, int z);
	vec3 getGlobalOffsetForNodePos(int x, int z);

	int lodForIndex(int i, int j);

	RenderData::IndexBuffer * getIndexBuffer(int size, tuple2i glue = tuple2i(0,0));
	
private://members

	typedef std::shared_ptr<RenderData::IndexBuffer> IB_PTR;

	int mStartLod;
	int mFirstLodStep;
	int mLodStep;
	int mMaxLod;
	
	static const int MAX_NODES_NUM = 41;
	std::auto_ptr<TerrainNode>	mNodes[MAX_NODES_NUM][MAX_NODES_NUM];
	std::auto_ptr<HeightMap>	mHMs[MAX_NODES_NUM][MAX_NODES_NUM];
	
	tuple2i mCenterNodePos;
	
	bool mGenerateMissingNodes;

	static const int MAX_TEXTURES_PER_NODE = TerrainNode::MAX_TEXTURES_PER_NODE;
	Resource::Texture * mTextures[MAX_TEXTURES_PER_NODE];
	Resource::Texture * mBumps[MAX_TEXTURES_PER_NODE];
	
	float mNodeSize;
	size_t mCellsPerNode;
	
	std::auto_ptr<FileSystem::FileStorage> mContentSource;
	
	static const int MAX_HM_SIZE_LOG2 = 10;
	static const int GLUE_AMPLITUDE = 3;
	IB_PTR mIndexBuffers[MAX_HM_SIZE_LOG2][GLUE_AMPLITUDE][GLUE_AMPLITUDE];
	
	AABB mBoundVolume;
	
	int mNodesNum;
	
	HeightGenerator mHeightGen;

	Resource::Program * mProgram;

	static Terrain * sMain;
};

}//namespace World { 
}//namespace Squirrel {
