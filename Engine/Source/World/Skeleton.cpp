#include "Skeleton.h"
#include "SceneObject.h"
#include <Common/Log.h>

namespace Squirrel {

namespace World { 

using namespace Math;

bool Skeleton::sCPUSkinning = true;

void Skeleton::EnableCPUSkinning(bool enable)
{
	sCPUSkinning = enable;
}

Skeleton::Skeleton()
{
	mVertexBuffer	= NULL;
}

Skeleton::~Skeleton(void)
{
	DELETE_PTR(mVertexBuffer);
}

void Skeleton::render(Render::IRender * render)
{
	return;

	render->enableDepthTest(Render::IRender::depthTestOff);
	render->setColor(vec4(0.5f, 0.99f, 0.9f, 1));

	int i, j;
	int vertsNum = mBones.getCount() * 2;
	int indsNum = mBones.getCount() * 2;

	VertexBuffer * vb = render->createVertexBuffer(VCI2VT(VertexBuffer::vcPosition), vertsNum);
	vb->setStorageType(IBuffer::stCPUMemory);
	IndexBuffer * ib = render->createIndexBuffer(indsNum);
	ib->setStorageType(IBuffer::stCPUMemory);
	ib->setPolyType(IndexBuffer::ptLines);

	std::vector<vec3> positions;

	for(i = 0, j = 0; i < mBones.getCount(); ++i, j+=2)
	{
		SceneObject * bone = mBones.getData()[ i ];
		SceneObject * parentBone = bone->getParent();
		if(parentBone == NULL) parentBone = bone;

		const mat4& boneTransform = bone->getTransform();
		vec3 pos = boneTransform.getTranslate();
		vb->setComponent<VertexBuffer::vcPosition>(j + 0, pos);
		ib->setIndex(j + 0, j + 0);

		positions.push_back(pos);

		const mat4& boneTransform2 = parentBone->getTransform();
		pos = boneTransform2.getTranslate();
		vb->setComponent<VertexBuffer::vcPosition>(j + 1, pos);
		ib->setIndex(j + 1, j + 1);
	}

	/*
	for(i = 1, j = 0; i < vertsNum; ++i)
	{
		int parentIndex = mBoneHierarchy.getData()[ i ];
		ib->setIndex(j++, parentIndex);
		ib->setIndex(j++, i);
	}
	*/

	render->setupVertexBuffer( vb );
	render->renderIndexBuffer( ib );

	DELETE_PTR(vb);
	DELETE_PTR(ib);

	render->enableDepthTest();

}

void Skeleton::buildGPUData()
{
	int i;

	int bonesDataSize = mBones.getCount() * 3;
	if(mGPUBonesData.getCount() != bonesDataSize)
	{
		mGPUBonesData.setCount(bonesDataSize);
	}

	int j = 0;
	for(i = 0; i < mBones.getCount(); ++i, j+=3)
	{
		SceneObject * bone = mBones.getData()[ i ];
		if(bone == NULL) continue;
		const mat4& currentTransform = bone->getTransform();
		mGPUBonesData.getData()[j+0] = currentTransform.x;
		mGPUBonesData.getData()[j+1] = currentTransform.y;
		mGPUBonesData.getData()[j+2] = currentTransform.z;
	}

	//invBind matrices are applied to mesh once instead of bones each time
}

void Skeleton::fetchBones(Resource::Skin * srcSkin, SceneObject * root)
{
	mBones.setCount( srcSkin->boneNames.getCount() );

	for(int i = 0; i < srcSkin->boneNames.getCount(); ++i)
	{
		const std::string& boneName = srcSkin->boneNames.getData()[i];

		SceneObject * bone = NULL;
		if(root->getName() == boneName)
		{
			bone = root;
		}
		else
		{
			bone = root->findChildWithName(boneName);
		}

		mBones.getData()[i] = bone;

		if(bone == NULL)
		{
			Log::Instance().error("Skeleton::fetchBones", (std::string("Can't find bone with name ") + boneName).c_str());
			continue;
		}
	}
}

void Skeleton::initVB(VertexBuffer * srcVB)
{
	if(sCPUSkinning)
	{
		//target vertex buffer
		Render::IRender * render = Render::IRender::GetActive();
		mVertexBuffer = render->createVertexBuffer(srcVB->getVertType(), srcVB->getVertsNum()); 
		mVertexBuffer->setStorageType(VertexBuffer::stGPUDynamicMemory);
	}
	else
	{
		srcVB->setStorageType(VertexBuffer::stGPUStaticMemory);
	}
}

void Skeleton::applyCPUSkinning(VertexBuffer * vb, Resource::Skin * skin)
{
	if(!sCPUSkinning) return;

	ASSERT( vb );
	ASSERT( skin );

	for(int i = 0; i < skin->joints.getCount(); ++i)
	{
		Skin::Joint& joint = skin->joints.getData()[ i ];

		vec3 srcPos = vb->getComponent<VertexBuffer::vcPosition>( i );
		vec3 srcNor = vb->getComponent<VertexBuffer::vcNormal>( i );
		vec2 srcTex = vb->getComponent<VertexBuffer::vcTexcoord>( i );

		vec3 pos(0,0,0);
		vec3 nor(0,0,0);

		for(int j = 0; j < joint.getCount(); ++j)
		{
			const tuple2i& boneInfluence = joint.getData()[ j ];
			float weight = skin->weights.getData()[ boneInfluence.x ];
			const mat4& boneTransfrom = mBones.getData()[ boneInfluence.y ]->getTransform();
			
			pos += (boneTransfrom * srcPos) * weight;
			nor += (boneTransfrom.getMat3() * srcNor) * weight;
		}

		mVertexBuffer->setComponent<VertexBuffer::vcPosition>( i,	pos );
		mVertexBuffer->setComponent<VertexBuffer::vcNormal>( i,	nor );
		mVertexBuffer->setComponent<VertexBuffer::vcTexcoord>( i,	srcTex );
	}

	mVertexBuffer->update( 0, mVertexBuffer->getVertsNum() * mVertexBuffer->getVertexSize() );
}

}//namespace World { 

}//namespace Squirrel {