#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "Light.h"
#include "ITexture.h"
#include "IProgram.h"
#include "Uniform.h"
#include <vector>
#include <list>
#include <set>
#include <common/ObjectsPool.h>

namespace Squirrel {

namespace Render { 

using namespace RenderData;

enum RenderQueueIndex//rename to RenderLayer?
{
	rqBackground	= 1000,
	rqGeometry		= 4000,
	rqClipped		= 6000,
	rqTransparent	= 8000,
	rqOverlay		= 10000,
	rqMax			= 20000,
};

static const int sDefaultRenderQueue = RenderQueueIndex::rqGeometry;

struct SQRENDER_API IndexPrimitive
{
	IndexPrimitive():
		mIB(NULL)
	{
		reset();
	}

	typedef std::vector<Math::mat4> TRANSFORMS_ARRAY;
	
	Math::AABB			mAABB;
	TRANSFORMS_ARRAY	mTransforms;
	IndexBuffer *		mIB;

	void addInstance(const Math::mat4& transform, const Math::AABB& bounds)
	{
		mTransforms.push_back(transform);
		mAABB.merge(bounds);
	}

	void reset()
	{
		mAABB.reset();
		mTransforms.clear();
	}
};


struct SQRENDER_API VBGroup {
	
	VBGroup():
		mVB(NULL), mBonesData(NULL), mBonesCount(0)
	{
		
	}
	~VBGroup()
	{
		clear();
	}

	typedef std::list<IndexPrimitive*>		INDEX_PRIMS_LIST;

	INDEX_PRIMS_LIST	mIndexPrimitives;

	VertexBuffer *		mVB;
	Math::vec4 *		mBonesData;
	int					mBonesCount;

	ObjectsPool<IndexPrimitive> * mIndexPrimitivesPool;

	//returns existed IndexPrimitive instance with the specified ib or new one if there is no such
	IndexPrimitive * getIndexPrimitive(IndexBuffer * ib);

	void reset()
	{
		clear();

		mBonesData = NULL;
		mBonesCount = 0;
	}

	void clear()
	{
		FOREACH(INDEX_PRIMS_LIST::iterator, itIP, mIndexPrimitives)
		{
			IndexPrimitive * ip = *itIP;

			ip->mTransforms.clear();

			mIndexPrimitivesPool->putObj(ip);
		}

		mIndexPrimitives.clear();
	}
};

struct ReflectionDesc
{
	vec3					mReflectionPos;
	vec3					mReflectionDir;

	ReflectionDesc(): mReflectionPos(0, 0, 0), mReflectionDir(0, 0, 0) {}

	bool operator==( const ReflectionDesc& rhs ) const
	{
		return mReflectionPos == rhs.mReflectionPos && mReflectionDir == rhs.mReflectionDir;
	}
	
	bool operator<( const ReflectionDesc& rhs ) const
	{
		for(int i = 0; i < 3; ++i)
		{
			if ( mReflectionPos[i] < rhs.mReflectionPos[i] )	return true;
			if ( mReflectionPos[i] > rhs.mReflectionPos[i] )	return false;
			if ( mReflectionDir[i] < rhs.mReflectionDir[i] )	return true;
			if ( mReflectionDir[i] > rhs.mReflectionDir[i] )	return false;
		}
		return false;
	}

};

struct SQRENDER_API MaterialGroup {

	typedef std::map<HashString, ITexture *> TEXTURES_MAP;

	typedef std::list<VBGroup*>		VB_GROUPS_LIST;

	MaterialGroup():
		mMaterial(NULL), mRequiresReflection(false),
		mCastsShadows(true), mReceivesShadows(true), mRenderQueue(sDefaultRenderQueue), mRenderOnce(false),
		mRequiresDepthBuffer(false), mRequiresColorBuffer(false)
	{
	
	}
	~MaterialGroup()
	{
		clear();
	}

	void clear()
	{
		FOREACH(VB_GROUPS_LIST::iterator, itVBGr, mVBGroups)
		{
			VBGroup * vbGr = *itVBGr;

			vbGr->clear();

			mVBGroupPool->putObj(vbGr);
		}

		mVBGroups.clear();
	}

	void reset()
	{
		clear();

		mProgramName = "";
		mProgramParams = "";
		mUniformsPool.clear();

		mMaterial = NULL;
		mTextures.clear();

		mRequiresReflection = false;

		mRenderQueue = sDefaultRenderQueue;

		mCastsShadows = true;

		mReceivesShadows = true;

		mRenderOnce = false;

		mRequiresColorBuffer = false;
		mRequiresDepthBuffer = false;
	}

	VB_GROUPS_LIST			mVBGroups;

	HashString				mProgramName;
	HashString				mProgramParams;
	
	bool					mRequiresReflection;//for reflective materials, e.g. water, mirror...
	bool					mRequiresColorBuffer;//for refractive materials, e.g. water, glass...
	bool					mRequiresDepthBuffer;

	Material *				mMaterial;
	UniformContainer		mUniformsPool;
	TEXTURES_MAP			mTextures;

	ReflectionDesc			mReflectionDesc;

	int						mRenderQueue;
	
	bool					mRenderOnce;
	
	bool					mReceivesShadows;
	bool					mCastsShadows;

	ObjectsPool<VBGroup> *	mVBGroupPool;
	ObjectsPool<IndexPrimitive> * mIndexPrimitivesPool;

	//returns existed VBGroup instance with the same vb or new one if there is no such
	VBGroup * getVBGroup(VertexBuffer * vb, int bonesCount = 0);

	bool sameMaterial(const MaterialGroup& otherMatGroup) const;
	bool sameProgram(const MaterialGroup& otherMatGroup) const;
	bool sameTextures(const MaterialGroup& otherMatGroup) const;
	bool sameTexture(const MaterialGroup& otherMatGroup, const HashString& texName) const;

	ITexture * getTexture(const UniformString& uniformName) const;

	static bool SortCriterion(const MaterialGroup& matGroup1, const MaterialGroup& matGroup2);
};

struct RenderOp {
	
	MaterialGroup *		mMaterialGroup;
	VBGroup *			mVBGroup;
	IndexPrimitive *	mIndexPrimitive;
};

class SQRENDER_API RenderQueue
{
public:

	RenderQueue(void);
	~RenderQueue(void);

	typedef std::list<Light*>				LIGHTS_LIST;
	typedef std::list<RenderOp>				RENDER_OPS_LIST;
	typedef std::set<ReflectionDesc>		REFL_DESCS_SET;

	LIGHTS_LIST& getLights() { return mLights; }

	REFL_DESCS_SET& getRequiredReflections() { return mRequiredReflections; }

	void put(Light* light);

	//returns temporary MaterialGroup to be filled with material data
	MaterialGroup *	beginMaterialGroup();
	//returns existed MaterialGroup with the same material as it was specified in temporary MaterialGroup instance
	//or newly added one (temporary MaterialGroup) if there is no such
	MaterialGroup * endMaterialGroup();

	//returns renderOpsList sorted by MatGroup->VBGroup, builds list if it is empty
	RENDER_OPS_LIST * getRenderOpsList();

	void clear();

private:

	typedef std::list<MaterialGroup*>		MAT_GROUPS_LIST;
	
	MAT_GROUPS_LIST				mMaterialGroups;

	RENDER_OPS_LIST				mRenderOpsList;

	REFL_DESCS_SET				mRequiredReflections;

	LIGHTS_LIST					mLights;

	MaterialGroup *				mTempMaterialGroup;

	ObjectsPool<MaterialGroup>	mMaterialGroupsPool;
	ObjectsPool<VBGroup>		mVBGroupsPool;
	ObjectsPool<IndexPrimitive>	mIndexPrimitivesPool;

};

}//namespace RenderData { 

}//namespace Squirrel {