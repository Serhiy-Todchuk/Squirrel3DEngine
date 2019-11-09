#include "RenderQueue.h"

namespace Squirrel {

namespace Render { 

IndexPrimitive * VBGroup::getIndexPrimitive(IndexBuffer * ib)
{
	FOREACH(INDEX_PRIMS_LIST::iterator, itIP, mIndexPrimitives)
	{
		IndexPrimitive * ip = *itIP;

		if(ip->mIB == ib)
		{
			return ip;
		}
	}

	IndexPrimitive * ret = mIndexPrimitivesPool->getObj();
	ret->reset();
	ret->mIB = ib;
	mIndexPrimitives.push_back(ret);
	return ret;
}

VBGroup *	MaterialGroup::getVBGroup(VertexBuffer * vb, int bonesCount)
{
	//consider that skinned vb is always different as bones almost always have unique transforms
	if(bonesCount == 0)
	{
		FOREACH(MaterialGroup::VB_GROUPS_LIST::iterator, itVBGr, mVBGroups)
		{
			VBGroup * vbGr = *itVBGr;

			if(vbGr->mVB == vb)
			{
				return vbGr;
			}
		}
	}

	//vb is unique
	VBGroup * ret = mVBGroupPool->getObj();
	ret->mIndexPrimitivesPool = mIndexPrimitivesPool;
	ret->reset();
	ret->mVB = vb;
	ret->mBonesCount = bonesCount;
	mVBGroups.push_back(ret);
	return ret;
}

ITexture * MaterialGroup::getTexture(const UniformString& uniformName) const
{
	TEXTURES_MAP::const_iterator itTex	= mTextures.find(uniformName);
	return itTex != mTextures.end() ? itTex->second : NULL;
}

bool MaterialGroup::sameProgram(const MaterialGroup& otherMatGroup) const
{
	if(mProgramName != otherMatGroup.mProgramName)
		return false;

	if(mProgramParams != otherMatGroup.mProgramParams)
		return false;

	return true;
}

bool MaterialGroup::SortCriterion(const MaterialGroup& matGroup1, const MaterialGroup& matGroup2)
{
	if(matGroup1.mRenderQueue < matGroup2.mRenderQueue)
		return true;
	if(matGroup1.mRenderQueue > matGroup2.mRenderQueue)
		return false;

	if(matGroup1.mProgramName.getHash() < matGroup2.mProgramName.getHash())
		return true;
	if(matGroup1.mProgramName.getHash() > matGroup2.mProgramName.getHash())
		return false;

	if(matGroup1.mProgramParams.getHash() < matGroup2.mProgramParams.getHash())
		return true;
	if(matGroup1.mProgramParams.getHash() > matGroup2.mProgramParams.getHash())
		return false;

	return false;
}

bool MaterialGroup::sameTextures(const MaterialGroup& otherMatGroup) const
{
	if(mTextures.size() != otherMatGroup.mTextures.size())
		return false;

	TEXTURES_MAP::const_iterator itTexOther	= otherMatGroup.mTextures.begin();
	TEXTURES_MAP::const_iterator itTexThis	= mTextures.begin();

	while(itTexOther != otherMatGroup.mTextures.end() && itTexThis != mTextures.end())
	{
		if(itTexOther->second != itTexThis->second)
			return false;

		++itTexOther;
		++itTexThis;
	}

	return true;
}

bool MaterialGroup::sameTexture(const MaterialGroup& otherMatGroup, const HashString& texName) const
{
	TEXTURES_MAP::const_iterator itTexOther	= otherMatGroup.mTextures.find(texName);
	TEXTURES_MAP::const_iterator itTexThis	= mTextures.find(texName);

	if(itTexOther == otherMatGroup.mTextures.end() || itTexThis == mTextures.end())
		return false;

	if(itTexOther->second != itTexThis->second)
		return false;

	return true;
}

bool MaterialGroup::sameMaterial(const MaterialGroup& otherMatGroup) const
{
	if(mRenderQueue != otherMatGroup.mRenderQueue)
		return false;

	if(!sameProgram(otherMatGroup))
		return false;

	if(!sameTextures(otherMatGroup))
		return false;
	
	if(!mUniformsPool.sameValues(otherMatGroup.mUniformsPool))
		return false;
	
	if(otherMatGroup.mMaterial	!= mMaterial)
		return false;

	if(mReceivesShadows != otherMatGroup.mReceivesShadows)
		return false;

	if(mRequiresReflection != otherMatGroup.mRequiresReflection)
		return false;

	if(mRequiresReflection && !(mReflectionDesc == otherMatGroup.mReflectionDesc))
		return false;

	return true;
}
	
RenderQueue::RenderQueue(void):
	mTempMaterialGroup(NULL)
{
}
	
RenderQueue::~RenderQueue(void)
{
	clear();
}

MaterialGroup *	RenderQueue::beginMaterialGroup()
{
	ASSERT(mTempMaterialGroup == NULL);//make sure prev begin is ended
	
	mTempMaterialGroup = mMaterialGroupsPool.getObj();
	mTempMaterialGroup->mVBGroupPool = &mVBGroupsPool;
	mTempMaterialGroup->mIndexPrimitivesPool = &mIndexPrimitivesPool;
	mTempMaterialGroup->reset();

	return mTempMaterialGroup;
}
	
MaterialGroup * RenderQueue::endMaterialGroup()
{
	ASSERT(mTempMaterialGroup != NULL);//make sure begin was called before

	MAT_GROUPS_LIST::const_iterator it = mMaterialGroups.begin();

	for(;it != mMaterialGroups.end(); ++it)
	{
		if(mTempMaterialGroup->sameMaterial(**it))
		{
			//material is very similiar
			mMaterialGroupsPool.putObj(mTempMaterialGroup);
			mTempMaterialGroup = NULL;
			return *it;
		}
	}

	//material is unique
	MaterialGroup * ret = mTempMaterialGroup;
	if(ret->mRequiresReflection)
		mRequiredReflections.insert(ret->mReflectionDesc);
	mMaterialGroups.push_back(ret);
	mTempMaterialGroup = NULL;
	return ret;
}

RenderQueue::RENDER_OPS_LIST * RenderQueue::getRenderOpsList()
{
	if(!mRenderOpsList.empty())
		return &mRenderOpsList;

	RenderOp renderOp;

	FOREACH(MAT_GROUPS_LIST::iterator, itMatGr, mMaterialGroups)
	{
		MaterialGroup * matGr = *itMatGr;

		renderOp.mMaterialGroup = matGr;

		FOREACH(MaterialGroup::VB_GROUPS_LIST::iterator, itVBGr, matGr->mVBGroups)
		{
			VBGroup * vbGr = *itVBGr;

			renderOp.mVBGroup = vbGr;

			FOREACH(VBGroup::INDEX_PRIMS_LIST::iterator, itIP, vbGr->mIndexPrimitives)
			{
				IndexPrimitive * ip = *itIP;

				renderOp.mIndexPrimitive = ip;

				mRenderOpsList.push_back(renderOp);
			}
		}
	}

	struct SortCriterion {
		bool operator() (const RenderOp& op1, const RenderOp& op2) {
			return MaterialGroup::SortCriterion(*op1.mMaterialGroup, *op2.mMaterialGroup);
		}
	} sortCriterion;

	mRenderOpsList.sort(sortCriterion);

	return &mRenderOpsList;
}

void RenderQueue::clear()
{
	FOREACH(MAT_GROUPS_LIST::iterator, itMatGr, mMaterialGroups)
	{
		MaterialGroup * matGr = *itMatGr;

		matGr->clear();

		mMaterialGroupsPool.putObj(matGr);
	}

	mMaterialGroups.clear();

	mLights.clear();

	mRenderOpsList.clear();

	mRequiredReflections.clear();
}

void RenderQueue::put(Light* light)
{
	if(light->mLightType == Light::ltDirectional)
		mLights.push_front(light);
	else
		mLights.push_back(light);
}

}//namespace Render{ 

}//namespace Squirrel {

