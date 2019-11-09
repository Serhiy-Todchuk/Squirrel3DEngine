// HeightMap.cpp: implementation of the HeightMap class.
//
//////////////////////////////////////////////////////////////////////

#include "HeightMap.h"

namespace Squirrel {
namespace World {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HeightMap::HeightMap(int xSize, int zSize, int dataBlockSize):
	mDataToDestroy(NULL)
{
	size_t memBlockSize = GetBlockSize(xSize, zSize, dataBlockSize);
	
	mHeader = new HeightMapHeader();
	
	mMemBlock = new char[memBlockSize];
	mMemOwner = true;
	
	mHeader->resolution = tuple2i(xSize, zSize);
	setupMemory();
	
	mHeader->dataElemSize = dataBlockSize;
	
	mHeader->texScale = vec2(1, 1);
}
	
HeightMap::HeightMap(HeightMapHeader * header, char * memBlock):
	mHeader(header), mMemBlock(memBlock), mMemOwner(false), mDataToDestroy(NULL)
{
	setupMemory();
}

HeightMap::~HeightMap()
{
	if(mMemOwner)
	{
		DELETE_ARR(mHeader);
		DELETE_ARR(mMemBlock);
	}
	DELETE_PTR(mDataToDestroy);
}
	
HeightMap * HeightMap::Load(Data * data)
{
	size_t headerSize = sizeof(HeightMapHeader);
	
	HeightMapHeader header;
	
	data->readBytes(&header, headerSize);
	
	HeightMap * hm = new HeightMap(header.resolution.x, header.resolution.y, header.dataElemSize);
	
	size_t memBlockSize = GetBlockSize(header.resolution.x, header.resolution.y, header.dataElemSize);

	data->readBytes(hm->mMemBlock, memBlockSize);
	
	hm->mHeader->resolution = tuple2i(header.resolution.x, header.resolution.y);
	hm->setupMemory();
	
	hm->mHeader->texScale = header.texScale;
	
	return hm;
}
	
HeightMap * HeightMap::LoadMapped(Data * data, bool takeCareOfData)
{
	HeightMapHeader * header = (HeightMapHeader *)data->getPtr();
	
	size_t headerSize = sizeof(HeightMapHeader);
	data->seekCur(headerSize);
	
	char * memBlock = (char *)data->getPtr();
	
	HeightMap * hm = new HeightMap(header, memBlock);
	
	size_t memBlockSize = hm->getBlockSize();
	data->seekCur(memBlockSize);
	
	if(takeCareOfData)
		hm->mDataToDestroy = data;
	
	return hm;
}
	
size_t HeightMap::GetBlockSize(int xSize, int zSize, int dataBlockSize)
{
	size_t elementsNum = xSize * zSize;
	return elementsNum * (sizeof(tuple4b) + sizeof(float) + dataBlockSize);
}
	
size_t HeightMap::getBlockSize() const
{
	return GetBlockSize(mHeader->resolution.x, mHeader->resolution.y, mHeader->dataElemSize);
}
	
void HeightMap::setupMemory()
{
	int elementsNum = mHeader->resolution.x * mHeader->resolution.y;
	
	int normalsSize = sizeof(tuple4b) * elementsNum;
	
	int heightsOffs = normalsSize;
	
	mNormals	= (tuple4b *)(mMemBlock);
	mHeights	= (float *)(mMemBlock + heightsOffs);
}

Data * HeightMap::save()
{
	size_t dataSize = getDataSize();
	Data * data = new Data(NULL, dataSize);
	
	if(save(data) < dataSize)
	{
		delete data;
		return NULL;
	}
	
	return data;
}
	
size_t HeightMap::save(Data * data)
{
	size_t headerSize = sizeof(HeightMapHeader);
	size_t memBlockSize = getBlockSize();
	
	data->putData(mHeader, headerSize);
	data->putData(mMemBlock, memBlockSize);
	
	return headerSize + memBlockSize;
}
	
void HeightMap::clear()
{
	size_t elementsNum = mHeader->resolution.x * mHeader->resolution.y;
	for(int i = 0; i < elementsNum; ++i)
	{
		mHeights[i] = 0;
		mNormals[i] = tuple4b(0, 127, 0, 255);
	}
}
	
size_t HeightMap::getDataSize() const
{
	return sizeof(HeightMapHeader) + getBlockSize();
}
	
float HeightMap::heightFetch(float x, float z) const
{
	float	xPos	= x;
	int		xNdx	= (int)xPos;
	float	xFrac	= xPos - xNdx;

	float	zPos	= z;
	int		zNdx	= (int)zPos;
	float	zFrac	= zPos - zNdx;

	float h1 = height(xNdx + 0, zNdx + 0);
	float h2 = height(xNdx + 1, zNdx + 0);
	float h3 = height(xNdx + 0, zNdx + 1);
	float h4 = height(xNdx + 1, zNdx + 1);

	float lerp1 = Math::lerp(h1, h2, xFrac);
	float lerp2 = Math::lerp(h3, h4, xFrac);

	return Math::lerp(lerp1, lerp2, zFrac);
}

vec3 HeightMap::normalFetch(float x, float z) const
{
	float	xPos	= x;
	int		xNdx	= (int)xPos;
	float	xFrac	= xPos - xNdx;

	float	zPos	= z;
	int		zNdx	= (int)zPos;
	float	zFrac	= zPos - zNdx;

	vec3 h1 = normalVec(xNdx + 0, zNdx + 0);
	vec3 h2 = normalVec(xNdx + 1, zNdx + 0);
	vec3 h3 = normalVec(xNdx + 0, zNdx + 1);
	vec3 h4 = normalVec(xNdx + 1, zNdx + 1);

	vec3 lerp1 = Math::lerp(h1, h2, xFrac);
	vec3 lerp2 = Math::lerp(h3, h4, xFrac);

	return Math::lerp(lerp1, lerp2, zFrac);
}
	
void HeightMap::updateNormals()
{
	for(int i = 0; i < getResolution().x; ++i)
	{
		for(int j = 0; j < getResolution().y; ++j)
		{
			updateNormal(i, j);
		}
	}
}
	
void HeightMap::updateNormal(int i, int j)
{
	float h = height(i, j);

	Math::vec3 n(0,0,0);

	bool pt1valid = i > 0;
	bool pt2valid = j > 0;
	bool pt3valid = (i + 1) < getResolution().x;
	bool pt4valid = (j + 1) < getResolution().y;

	Math::vec3 pt1(-1,	(pt1valid ? height(i-1,j) : h) - h,	0);
	Math::vec3 pt2(0,	(pt2valid ? height(i,j-1) : h) - h,	-1);
	Math::vec3 pt3(+1,	(pt3valid ? height(i+1,j) : h) - h,	0);
	Math::vec3 pt4(0,	(pt4valid ? height(i,j+1) : h) - h,	+1);

	if(pt2valid && pt1valid)
		n += pt2 ^ pt1;
	if(pt3valid && pt2valid)
		n += pt3 ^ pt2;
	if(pt4valid && pt3valid)
		n += pt4 ^ pt3;
	if(pt1valid && pt4valid)
		n += pt1 ^ pt4;

	n.normalize();

	tuple4b packedNormal(0,0,0,0);
	packedNormal.setNormalized(n.x, 0);
	packedNormal.setNormalized(n.y, 1);
	packedNormal.setNormalized(n.z, 2);
	normalRef(i,j) = packedNormal;
}

}//namespace World {
}//namespace Squirrel {
