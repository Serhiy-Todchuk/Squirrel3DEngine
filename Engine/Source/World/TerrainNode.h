#pragma once

#include "HeightMap.h"
#include <Render/RenderQueue.h>
#include <Render/Camera.h>
#include <Resource/TextureStorage.h>
#include "macros.h"

namespace Squirrel {
namespace World { 

using namespace Math;

class SQWORLD_API TerrainNode
{
public:
	static const int MAX_TEXTURES_PER_NODE = 4;

public://ctor/dtor
	TerrainNode();
	virtual ~TerrainNode();

public://methods

	void init(RenderData::IndexBuffer * ib, HeightMap *	heightMap, int lod = 0);

	float height(float x, float z) const;

	bool render(Render::Camera * camera, Render::MaterialGroup * matGroup);

	inline void setOffset(vec3 offset) { mOffset = offset; mUpdateBoundVolume = true; }
	inline void setScale(vec3 scale) { mScale = scale; mUpdateBoundVolume = true; }

	inline vec3 getOffset() const { return mOffset; }
	inline vec3 getScale() const { return mScale; }

	inline int getLOD() const { return mLod; }

	inline const AABB& getTransformedAABB()
	{
		if(mUpdateBoundVolume)
		{
			mTransformedBoundVolume = mBoundVolume;
			mTransformedBoundVolume.scale(mScale);
			mTransformedBoundVolume.move(mOffset);
		}
		return mTransformedBoundVolume;
	}

	inline void setIndexBuffer(RenderData::IndexBuffer * ib, bool own = false) { 
		if(mIBOwner) 
			DELETE_PTR(mIB); 
		mIB = ib; 
		mIBOwner = own;
	}
	
	inline void setGridPos(tuple2i pos) { mGridPos = pos; }
	inline tuple2i getGridPos() const { return mGridPos; }

	inline HeightMap * getHeightMap() const { return mHeightMap; }
	
	static RenderData::IndexBuffer * GenIndexBuffer(tuple2i size, tuple2i glue = tuple2i(0, 0));
	static RenderData::VertexBuffer * GenVertexBuffer(tuple2i size);

	Resource::Texture * mTextures[MAX_TEXTURES_PER_NODE];
	Resource::Texture * mBumps[MAX_TEXTURES_PER_NODE];
	
private://methods

	void applyHeightMap(HeightMap *	heightMap, int lod);
	
private://members
	
	tuple2i		mGridPos;

	vec3		mOffset;
	vec3		mScale;
	HeightMap *	mHeightMap;

	int			mLod;

	AABB		mBoundVolume;
	AABB		mTransformedBoundVolume;
	bool		mUpdateBoundVolume;

	RenderData::VertexBuffer * mVB;
	RenderData::IndexBuffer * mIB;
	
	bool mIBOwner;
};

}//namespace World {
}//namespace Squirrel {
