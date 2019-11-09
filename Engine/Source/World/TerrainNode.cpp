// TerrainNode.cpp: implementation of the TerrainNode class.
//
//////////////////////////////////////////////////////////////////////

#include "TerrainNode.h"

namespace Squirrel {
namespace World {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TerrainNode::TerrainNode()
{
	mOffset = vec3::Zero();
	mScale = vec3::One();
	mHeightMap = NULL;

	mVB = NULL;
	mIB = NULL;
	mIBOwner = false;
	
	mGridPos = tuple2i(0, 0);
}

TerrainNode::~TerrainNode()
{
	DELETE_PTR( mVB );
	if(mIBOwner)
		DELETE_PTR( mIB );
}

using RenderData::VertexBuffer;
using RenderData::IndexBuffer;

void TerrainNode::init(RenderData::IndexBuffer * ib, HeightMap *	heightMap, int lod)
{
	mLod = lod;

	tuple2i hmSize = heightMap->getResolution();

	int lodPow2 = 1 << lod;
	hmSize = (hmSize - 1) / lodPow2 + 1;
	
	//gen vertex buffer
	
	mVB = GenVertexBuffer(hmSize);
	mVB->setStorageType(VertexBuffer::stGPUStaticMemory);
	
	applyHeightMap(heightMap, lod);

	if(ib == NULL)
	{
		mIB = NULL;
		mIBOwner = false;
	}
	else
	{
		mIB = ib;
		mIBOwner = false;
	}
}
	
RenderData::IndexBuffer * TerrainNode::GenIndexBuffer(tuple2i size, tuple2i glue)
{
	Render::IRender * render = Render::IRender::GetActive();
	
	uint indsNum = (size.x - 1) * (size.y * 2) + (size.x - 2) * 2;
	RenderData::IndexBuffer * ib = render->createIndexBuffer(indsNum);
	ib->setStorageType(RenderData::IBuffer::stGPUStaticMemory);
	ib->setPolyType(IndexBuffer::ptTriStrip);
	
	int indexIndex = 0;
	for(int i = 0; i < size.x - 1; ++i)
	{
		if(i > 0)
		{
			int left = ((i + 0) * size.x);
			if(glue.y < 0 && (i % 2) != 0)
			{
				left = ((i - 1) * size.x);
			}

			ib->setIndex(indexIndex++, left);
		}
		
		for(int j = 0; j < size.y; ++j)
		{
			int left = ((i + 0) * size.x) + j;

			if(glue.y < 0 && j == 0 && (i % 2) != 0)
			{
				left = ((i - 1) * size.x);
			}
			if(glue.y > 0 && j == (size.y - 1) && (i % 2) != 0)
			{
				left = ((i - 1) * size.x) + j;
			}

			if(glue.x < 0 && (j % 2) != 0 && i == 0)
			{
				left = ((i + 0) * size.x) + (j - 1);
			}

			int right = ((i + 1) * size.x) + j;

			if(glue.y < 0 && j == 0 && ((i + 1) % 2) != 0)
			{
				right = ((i + 0) * size.x);
			}
			if(glue.y > 0 && j == (size.y - 1) && ((i + 1) % 2) != 0)
			{
				right = ((i + 0) * size.x) + j;
			}

			if(glue.x > 0 && (j % 2) != 0 && i == (size.x - 2))
			{
				right = ((i + 1) * size.x) + (j - 1);
			}
			
			ib->setIndex(indexIndex++, left);
			ib->setIndex(indexIndex++, right);
		}
		
		if(i < size.x - 2)
		{
			int right = ((i + 1) * size.x) + (size.y - 1);
			if(glue.y > 0 && ((i + 1) % 2) != 0)
			{
				right = ((i + 0) * size.x) + (size.y - 1);
			}

			ib->setIndex(indexIndex++, right);
		}
	}
	
	return ib;
}
	
RenderData::VertexBuffer * TerrainNode::GenVertexBuffer(tuple2i size)
{
	//gen vertex buffer
	
	uint vertsNum = size.x * size.y;
	
	//position, packedNormal...
	int terrainVertexType =	(VCI2VT(VertexBuffer::vcPosition) |
							 VCI2VT(VertexBuffer::vcInt8Normal));
	
	Render::IRender * render = Render::IRender::GetActive();
	return render->createVertexBuffer(terrainVertexType, vertsNum);
}
	
void TerrainNode::applyHeightMap(HeightMap * heightMap, int lod)
{
	mHeightMap = heightMap;

	mBoundVolume.reset();
	
	int i, j, x, y;
	
	tuple2i hmSize = heightMap->getResolution();

	int lodPow2 = 1 << lod;
	tuple2i lodSize = (hmSize - 1) / lodPow2 + 1;

	//gen vertex buffer
	
	int vertexIndex = 0;
	for(i = 0, x = 0; i < lodSize.x; ++i, x += lodPow2)
	{
		for(j = 0, y = 0; j < lodSize.y; ++j, y += lodPow2)
		{
			//position
			float h = heightMap->height(x, y);
			Math::vec3 vertPos((float)x, h, (float)y);
			mVB->setComponent<VertexBuffer::vcPosition>(vertexIndex, vertPos);
			
			//normal
			mVB->setComponent<VertexBuffer::vcInt8Normal>(vertexIndex, heightMap->normal(x, y));
			
			//update BV
			mBoundVolume.addVertex( vertPos );
			
			++vertexIndex;
		}
	}
	
	mUpdateBoundVolume = true;
}
	
float TerrainNode::height(float x, float z) const
{
	x -= mOffset.x;
	z -= mOffset.z;
	x /= mScale.x;
	z /= mScale.z;
	return mHeightMap->heightFetch(x,z) * mScale.y + mOffset.y;
}

bool TerrainNode::render(Render::Camera * camera, Render::MaterialGroup * matGroup)
{
	const AABB& bounds = getTransformedAABB();

	if(!camera->isAABBIn(bounds))
	{
		return false;
	}

	Render::VBGroup * vbGroup = matGroup->getVBGroup(mVB, 0);

	Render::IndexPrimitive * primitive = vbGroup->getIndexPrimitive(mIB);

	Math::mat4 transform = Math::mat4::Scale(mScale);
	transform.setTranslate(mOffset);

	primitive->addInstance(transform, bounds);

	return true;
}

}//namespace World {
}//namespace Squirrel {
