#pragma once

#include <Math/mathTypes.h>
#include <Common/BufferArray.h>
#include <Render/IRender.h>
#include <Resource/Mesh.h>
#include <Resource/Skin.h>
#include "SceneObject.h"
#include "macros.h"

namespace Squirrel {

namespace World { 

using namespace RenderData;

class SQWORLD_API Skeleton
{
public:
	Skeleton(void);
	~Skeleton(void);

	BufferArray<SceneObject *> *	getBones() { return &mBones; }

	//void reset();
	//animate
	void fetchBones(Resource::Skin * srcSkin, SceneObject * root);
	void buildGPUData();
	void applyCPUSkinning(VertexBuffer * vb, Resource::Skin * srcSkin);

	void render(Render::IRender * render);

	void initVB(VertexBuffer * srcVB);

	inline VertexBuffer *	getVertexBuffer()	{ return mVertexBuffer; }

	inline BufferArray<Math::vec4>& getGPUBonesData() { return mGPUBonesData; }

	static void EnableCPUSkinning(bool enable);

private:

	//bones
	BufferArray<SceneObject *>	mBones;

	//prepared bones data
	BufferArray<Math::vec4> mGPUBonesData;

	//target vertex buffer
	VertexBuffer *	mVertexBuffer;

	static bool sCPUSkinning;
};


}//namespace World { 

}//namespace Squirrel {