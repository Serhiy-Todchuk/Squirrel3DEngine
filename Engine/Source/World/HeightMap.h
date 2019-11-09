#pragma once

#include <common/common.h>
#include <Resource/Mesh.h>
#include <Render/IRender.h>
#include "macros.h"

namespace Squirrel {
namespace World { 

using namespace Math;
	
#pragma pack(push, 1)
	
struct HeightMapHeader {
	static const int sTextureNameMaxSize	= 128;
	static const int sTexturesPerNode		= 4;
	
	tuple2i		resolution;
	uint32		dataElemSize;
	vec2		texScale;
	char		textureNames[sTexturesPerNode][sTextureNameMaxSize];
};
	
#pragma pack(pop)

//proxy interface for physic engine
class SQWORLD_API HeightMap
{
public://ctor/dtor
	HeightMap(int xSize, int zSize, int dataBlockSize = 0);
	HeightMap(HeightMapHeader * header, char * memBlock);
	virtual ~HeightMap();

public://methods
	inline float	height(int xNdx, int zNdx) const	{ return mHeights[xNdx + (zNdx * mHeader->resolution.x)]; }
	inline tuple4b	normal(int xNdx, int zNdx) const	{ return mNormals[xNdx + (zNdx * mHeader->resolution.x)]; }
	
	inline float&	heightRef(int xNdx, int zNdx)	{ return mHeights[xNdx + (zNdx * mHeader->resolution.x)]; }
	inline tuple4b&	normalRef(int xNdx, int zNdx)	{ return mNormals[xNdx + (zNdx * mHeader->resolution.x)]; }
	
	inline char *	dataPtr(int xNdx, int zNdx)	{ return &mData[(xNdx + (zNdx * mHeader->resolution.x)) * mHeader->dataElemSize]; }
	
	inline vec3		normalVec(int xNdx, int zNdx) const	{
		tuple4b n = normal(xNdx, zNdx);
		return vec3( (float(n.x)/127.5f)-1, (float(n.y)/127.5f)-1, (float(n.z)/127.5f)-1 );
	}

	float heightFetch(float x, float z) const;
	vec3 normalFetch(float x, float z) const;

	inline void	setTexScale(vec2 texScale)	{ mHeader->texScale = texScale; }
	inline vec2	getTexScale() const			{ return mHeader->texScale; }

	inline tuple2i	getResolution()	const	{ return mHeader->resolution; }

	//generating/editing/loading stuff
	void updateNormal(int i, int j);
	void updateNormals();

	inline float *		getHeights()	const	{ return mHeights; }
	inline tuple4b *	getNormals()	const	{ return mNormals; }
	
	static HeightMap * Load(Data * data);
	static HeightMap * LoadMapped(Data * data, bool takeCareOfData = false);
	Data * save();
	size_t save(Data * data);
	
	size_t getDataSize() const;
	
	void clear();

private:
	
	static size_t GetBlockSize(int xSize, int zSize, int dataBlockSize);
	void setupMemory();
	size_t getBlockSize() const;
	
private:

	HeightMapHeader * mHeader;
	
	tuple4b *	mNormals;
	float	*	mHeights;
	char	*	mData;
	
	char	*	mMemBlock;
	bool		mMemOwner;
	
	Data	*	mDataToDestroy;
};

}//namespace World { 
}//namespace Squirrel {
